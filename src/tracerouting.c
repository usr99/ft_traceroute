/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tracerouting.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/24 14:19:19 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/26 18:16:43 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <linux/ip.h>
#include <linux/icmp.h>
#include <stdio.h>
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
	fd_set rfds;
	struct timeval timeout = {
		.tv_sec = (int)cfg->opt.waittime,
		.tv_usec = (cfg->opt.waittime - (int)cfg->opt.waittime) * 1000000
	};
	int count = 0;

	while (count < cfg->opt.squeries)
	{
		/*
		** Block until there is input to read from ICMP socket
		** or until the timeout expires (defined by -w option)
		*/
		int ret;
		FD_ZERO(&rfds);
		FD_SET(cfg->icmp_sockfd, &rfds);
		if ((ret = select(cfg->icmp_sockfd + 1, &rfds, NULL, NULL, &timeout)) == -1)
			return -1;
		else if (ret == 0)
			return 0; // no replies after timeout

		/* Receive ICMP message */
		char buf[100];
		struct sockaddr_storage address;
		socklen_t addrlen = sizeof(address);
		ssize_t bytes = recvfrom(cfg->icmp_sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&address, &addrlen);
		if (bytes == -1)
			return -1;

		int destination = process_response(buf, bytes, route->hops, cfg);
		if (destination == -1)
			return -1; // some kind of error
		else
		{
			/*
			** Check that we reached our destination host
			** we only store the lowest number of hops found
			*/
			if (destination && route->maxlen > destination)
				route->maxlen = destination;
			count++;
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
			if (ft_strlen(p->address))
			{
				if (!gateway || ft_strncmp(gateway, p->address, INET6_ADDRSTRLEN) != 0)
					printf("%s (%s) ", p->hostname, p->address);
				printf(" %.3f ms ", get_duration_ms(&p->time_sent, &p->time_recvd));
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
