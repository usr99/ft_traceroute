/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tracerouting.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 14:19:19 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/27 00:37:16 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <linux/ip.h>
#include <linux/icmp.h>
#include <stdio.h>
#include <errno.h>

#include "ft_traceroute.h"

int send_probes(t_config* cfg, t_route* route)
{
	t_hop* current_hop = route->hops + (route->current_ttl - cfg->opt.first_ttl);
	int total = 0;

	while (total < cfg->opt.squeries)
	{
		if (current_hop->probes == NULL)
		{
			if (route->current_ttl > cfg->opt.max_ttl)
				break;

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

		total++;
		current_hop->nb_sent++;

		/* Increase ttl after sending all probes for current hop */
		if (current_hop->nb_sent == cfg->opt.nqueries)
		{
			current_hop++;
			route->current_ttl++;
		}
	}
	return 0;
}

int recv_response(t_config* cfg, t_route* route)
{
	char buf[100];
	struct sockaddr_storage address;
	socklen_t addrlen;
	ssize_t bytes;

	int destination;
	uint8_t count = 0;
	while (count < cfg->opt.squeries)
	{
		addrlen = sizeof(address);
		bytes = recvfrom(cfg->icmp_sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&address, &addrlen);
		if (bytes == -1)
		{
			if (errno != EAGAIN)
				return -1;

			check_timeout(cfg, route, &count);
		}
		else
		{
			destination = process_response(buf, bytes, route->hops, cfg);
			if (destination == -1)
				return -1; // some kind of error
			else
			{
				/*
				** Check that we reached our destination host
				** we only store the lowest number of hops found
				*/
				if (destination && route->maxlen > destination)
					route->maxlen = destination - 1;
				count++;
			}
		}
	}

	return 0;	
}

bool browse_route(t_config* cfg, t_route* route)
{
	t_hop* last_hop = route->hops + route->len;

	while (last_hop->nb_sent == cfg->opt.nqueries && route->len <= route->maxlen)
	{
		printf(" %d  ", route->len + cfg->opt.first_ttl);

		t_probe* p;
		const char* gateway = NULL;
		for (p = last_hop->probes; p - last_hop->probes < last_hop->nb_sent; p++)
		{
			if (p->status == SUCCESS)
			{
				if (!gateway || ft_strncmp(gateway, p->address, INET6_ADDRSTRLEN) != 0)
					printf("%s (%s) ", p->hostname, p->address);
				printf(" %.3f ms ", p->rtt);
			}
			else
				printf("* ");
			gateway = p->address;
		}
		printf("\n");

		last_hop++;
		route->len++;

		if (route->len > route->maxlen)
			return true; // route is complete
	}
	return false; // keep tracing the route
}

void check_timeout(t_config* cfg, t_route* route, uint8_t *count)
{
	float near = -1.f;
	float here;
	float elapsed_time;

	t_probe* probe;
	int i;
	unsigned int j;

	for (i = route->current_ttl - cfg->opt.first_ttl; i >= route->len; i--)
	{		
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

				if (here != -1.f && timeout > here * cfg->opt.timeout.here)
					timeout = here * cfg->opt.timeout.here;
				if (near != -1.f && timeout > near * cfg->opt.timeout.near)
					timeout = near * cfg->opt.timeout.near;

				if (elapsed_time > timeout)
				{
					probe->status = TIMED_OUT;
					(*count)++;
				}
			}
		}
	}
}

