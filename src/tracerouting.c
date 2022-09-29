/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tracerouting.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 14:19:19 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/29 14:27:37 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <linux/ip.h>
#include <linux/icmp.h>
#include <stdio.h>
#include <errno.h>

#include "ft_traceroute.h"

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
		if (setsockopt(cfg->sockfd, IPPROTO_IP, IP_TTL, &route->current_ttl, sizeof(uint8_t)) == -1)
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

	init_probe(current_hop->probes + current_hop->nb_sent, cfg->opt.port); 
	if (sendto(cfg->sockfd, NULL, 0, 0, (struct sockaddr*)&cfg->host, sizeof(struct sockaddr_storage)) >= 0)
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
	char buf[100];
	struct sockaddr_storage address;
	socklen_t addrlen;
	ssize_t bytes;

	/* Try to receive a response to one of the probes sent */
	addrlen = sizeof(address);
	bytes = recvfrom(cfg->icmp_sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&address, &addrlen);
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
	int type = validate_packet(buf, bytes, cfg);
	if (type == 0)
		return 0; // ignore packet

	/*
	** Extract useful information from the packet
	** the destination port used in the original datagram
	** and the source ip address
	*/
	uint16_t id;
	struct sockaddr_in addr;
	parse_packet(buf, &addr, &id);

	/* Match the id found with our probes */
	size_t probeidx = (id - route->hops->probes->id);
	t_hop* hop = route->hops + (probeidx / cfg->opt.nqueries);
	t_probe* gateway = hop->probes + (probeidx % cfg->opt.nqueries);
	
	/* Update probe status */
	gateway->status = SUCCESS;
	gateway->rtt = get_duration_from_now(&gateway->time_sent);
	if (type == ICMP_PORT_UNREACH)
		hop->is_destination = true;

	/* Save source address as a string and perform a reverse dns lookup to retrieve hostname if any */
	if (inet_ntop(AF_INET, &addr.sin_addr.s_addr, gateway->address, INET6_ADDRSTRLEN) == NULL)
		return -1;
	// if (getnameinfo((struct sockaddr*)&addr, sizeof(struct sockaddr_in), gateway->hostname, HOST_NAME_MAX, NULL, 0, 0) != 0)
	// {
		ft_strlcpy(gateway->hostname, gateway->address, INET6_ADDRSTRLEN); // copy ip address if dns lookup fails
	// }

	hop->nb_recvd++;
	(*nprobes)--;
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
			else if (probe->status != TIMED_OUT)
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
			if (hop->probes[i].status == WAITING_REPLY)
				return false; // we are still waiting for responses
		}

		/* Log hop statistics */
		t_probe* last = NULL;
		printf(" %d  ", route->len + cfg->opt.first_ttl);
		for (i = 0; i < cfg->opt.nqueries; i++)
		{
			if (hop->probes[i].status == SUCCESS)
			{
				/* Print gateway address only if it's not the same than the last probe from same hop */
				if (!last || ft_strncmp(last->address, hop->probes[i].address, INET6_ADDRSTRLEN) != 0)
					printf("%s (%s) ", hop->probes[i].hostname, hop->probes[i].address);
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
