/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tracerouting.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 14:19:19 by mamartin          #+#    #+#             */
/*   Updated: 2022/10/03 01:25:58 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/icmp6.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <errno.h>

#include "ft_traceroute.h"
#include "dns.h"

int send_probes(t_config* cfg, t_route* route, unsigned int *nprobes)
{
	static bool all_probes_sent = false;

	if (*nprobes >= cfg->opt.squeries || all_probes_sent)
		return 0;

	t_hop* current_hop = route->hops + (route->current_ttl - cfg->opt.first_ttl);

	if (current_hop->probes == NULL) // no probes were sent for this hop
	{
		/* Allocate a new array of probes for the next hop */
		current_hop->probes = ft_calloc(cfg->opt.nqueries, sizeof(t_probe));
		if (!current_hop->probes)
			return -1;

		/* Set ttl field for the future packets */
		int ret;
		if (cfg->opt.family == AF_INET)
			ret = setsockopt(cfg->sockfd, IPPROTO_IP, IP_TTL, &route->current_ttl, sizeof(int));
		else
			ret = setsockopt(cfg->sockfd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &route->current_ttl, sizeof(int));
		if (ret == -1)
			return -1;
	}

	/*
	** Set destination port
	** we don't want the host to process the probe
	** so we send it on an "unlikely" port
	*/
	if (cfg->opt.family == AF_INET)
		((struct sockaddr_in*)&cfg->host)->sin_port = htons(cfg->opt.port);
	else
		((struct sockaddr_in6*)&cfg->host)->sin6_port = htons(cfg->opt.port);

	char buffer[PACKETLEN_MAX] = {0};
	size_t bufsize = cfg->opt.packetlen - (cfg->opt.family == AF_INET ? PACKETLEN_V4_MIN : PACKETLEN_V6_MIN);
	init_probe(current_hop->probes + current_hop->nb_sent, cfg->opt.port); 
	if (sendto(cfg->sockfd, buffer, bufsize, 0, (struct sockaddr*)&cfg->host, sizeof(struct sockaddr_storage)) >= 0)
		cfg->opt.port++; // increased for each probe to match them with responses
	else
		return -1;
	(*nprobes)++;

	/* Increase ttl after sending all probes for current hop */
	current_hop->nb_sent++;
	if (current_hop->nb_sent == cfg->opt.nqueries)
	{
		if (route->current_ttl == cfg->opt.max_ttl)
			all_probes_sent = true;
		else
			route->current_ttl++;
		current_hop++;
	}

	return 0;
}

int recv_response(t_config* cfg, t_route* route, unsigned int *nprobes)
{
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(struct sockaddr_storage);
	char buf[100];
	ssize_t bytes;

	/* Try to receive a response to one of the probes sent */
	bytes = recvfrom(cfg->icmp_sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addrlen);
	if (bytes == -1) // no response
	{
		if (errno != EAGAIN)
			return -1; // failure
		*nprobes -= check_timeout(cfg, route);
		return 0;
	}

	/*
	** Raw sockets receive a copy of every ICMP messages
	** so we need to check that this packet really belongs to us
	*/
	uint8_t type = validate_packet(buf, bytes, cfg);
	if (type == 0)
		return 0; // ignore packet

	/* Match the id found with our probes */
	uint16_t id = extract_probe_id(buf, &addr);
	size_t probeidx = (id - route->hops->probes->id);
	t_hop* hop = route->hops + (probeidx / cfg->opt.nqueries);
	t_probe* gateway = hop->probes + (probeidx % cfg->opt.nqueries);
	
	/* Update probe */
	gateway->rtt = get_duration_from_now(&gateway->time_sent);
	if ((cfg->opt.family == AF_INET && type == ICMP_PORT_UNREACH) || (cfg->opt.family == AF_INET6 && type == ICMP6_DST_UNREACH_NOPORT))
		hop->is_destination = true;
	if (addr_to_text(&addr, gateway->address) == -1) // save source address as a string
		return -1;
	if (cfg->opt.dns_enabled) // reverse dns on the gateway address that replied to us
	{
		gateway->status = WAITING_NAME_INFO;
		if (reverse_dns_lookup(&addr, gateway, cfg, nprobes) == -1)
			return -1;
	}
	else
	{
		(*nprobes)--;
		gateway->status = SUCCESS;
	}

	hop->nb_recvd++;
	return 0;
}

int check_timeout(t_config* cfg, t_route* route)
{
	float near = -1.f;
	float here;
	float elapsed_time;

	t_probe* probe;
	int i;
	unsigned int j;
	int count = 0;

	for (
		i = route->current_ttl - cfg->opt.first_ttl;
		i >= route->len;
		i--
	) {
		here = -1.f;
		for (j = 0; j < route->hops[i].nb_sent; j++)
		{
			probe = route->hops[i].probes + j;
			if (probe->status == SUCCESS)
			{
				if (here == -1.f || probe->rtt < here)
					here = probe->rtt;
				if (near == -1.f || probe->rtt < near)
					near = probe->rtt;
			}
			else if (probe->status != TIMED_OUT && probe->status != WAITING_NAME_INFO)
			{
				float timeout = cfg->opt.timeout.max * 1000.f;
				elapsed_time = get_duration_from_now(&probe->time_sent);

				if (here != -1.f && cfg->opt.timeout.here && timeout > here * cfg->opt.timeout.here)
					timeout = here * cfg->opt.timeout.here;
				if (near != -1.f && cfg->opt.timeout.near && timeout > near * cfg->opt.timeout.near)
					timeout = near * cfg->opt.timeout.near;

				if (elapsed_time > timeout)
				{
					probe->status = TIMED_OUT;
					count++;
				}
			}
		}
	}
	return count;
}

bool browse_route(t_config* cfg, t_route* route)
{
	const t_hop* hop = route->hops + route->len;

	if (hop->nb_sent == cfg->opt.nqueries)
	{
		/* Check that all probes are either successful or timed out */
		int i;
		for (i = 0; i < cfg->opt.nqueries; i++)
		{
			if (hop->probes[i].status == WAITING_REPLY || hop->probes[i].status == WAITING_NAME_INFO)
				return false; // we are still waiting for responses
		}

		/* Log hop statistics */
		t_probe* last = NULL;
		if (route->len < 9)
			ft_putchar_fd(' ', STDOUT_FILENO);
		printf("%d  ", route->len + cfg->opt.first_ttl);
		for (i = 0; i < cfg->opt.nqueries; i++)
		{
			if (hop->probes[i].status == SUCCESS)
			{
				/* Print gateway address only if it's not the same than the last probe from same hop */
				if (!last || ft_strncmp(last->address, hop->probes[i].address, INET6_ADDRSTRLEN) != 0)
				{
					if (hop->probes[i].hostname[0] != '\0')
						printf("%s (%s) ", hop->probes[i].hostname, hop->probes[i].address);
					else
						printf("%s ", hop->probes[i].address);
				}
				printf(" %.3f ms ", hop->probes[i].rtt);
			}
			else
				printf("* ");
			last = hop->probes + i;
		}
		printf("\n");

		route->len++;
		if (hop->is_destination)
			return hop->is_destination;
	}
	return route->len == route->maxlen;
}
