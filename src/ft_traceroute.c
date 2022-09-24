/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/18 10:13:43 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/23 21:54:10 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/time.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <errno.h>

#include "ft_traceroute.h"
#include "probes.h"
#include "dns.h"


int main(int argc, char** argv)
{
	t_config config;
	if (setup_tracerouting(argc, argv, &config) == -1)
		return 2;

	t_route route;
	if (init_route(&config, &route) == -1)
	{
		close(config.sockfd);
		close(config.icmp_sockfd);
		return 2;
	}

	while (route.last_ttl <= config.opt.max_ttl)
	{
		if (send_probes(&config, &route) != 0)
		{
			// behavior not defined yet
			return 2;
		}

		if (recv_response(&config, &route) != 0)
		{
			// behavior not defined yet
			return 2;
		}
		// debug_route(&route, &config);

		// debug_route(&route, &config);
		log_route(&config, &route);

		break ;
	}

	close(config.sockfd);
	close(config.icmp_sockfd);
	destroy_route(&route);
	return 0;
}

int setup_tracerouting(int argc, char** argv, t_config* cfg)
{
	if (parse_arguments(argc, argv, &cfg->opt) == -1)
		return -1;

	if (getuid() != 0)
		return log_error("user must have root privileges !");

	if (resolve_hostname(&cfg->host, &cfg->opt) == -1)
		return -1;

	if ((cfg->sockfd = socket(cfg->host.ai_family, cfg->host.ai_socktype, cfg->host.ai_protocol)) == -1)
		return log_error("failed to create UDP socket");

	if ((cfg->icmp_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
	{
		close(cfg->sockfd);
		return log_error("failed to create raw socket");
	}

	/* Only accepts "Destination Unreachable" and "Time Exceeded" */
	struct icmp_filter filter = { .data = ~(1 << ICMP_DEST_UNREACH | 1 << ICMP_TIME_EXCEEDED) };
	if (setsockopt(cfg->icmp_sockfd, IPPROTO_RAW, ICMP_FILTER, &filter, sizeof(struct icmp_filter)) != 0)
	{
		close(cfg->sockfd);
		close(cfg->icmp_sockfd);
		return log_error("failed to set ICMP filter");
	}
	return 0;
}

int init_route(t_config* cfg, t_route* route)
{
	char address[INET6_ADDRSTRLEN];
	if (inet_ntop(cfg->host.ai_family, &((struct sockaddr_in*)cfg->host.ai_addr)->sin_addr, address, INET6_ADDRSTRLEN) == NULL)
		return -1;

	route->len = 0;
	route->last_ttl = cfg->opt.first_ttl;
	route->maxlen = cfg->opt.max_ttl - cfg->opt.first_ttl + 1;

	route->hops = ft_calloc(route->maxlen, sizeof(t_hop));
	if (!route->hops)
		return log_error("Out of memory");

	printf("traceroute to %s (%s), %d hops max, %d byte packets\n",
		cfg->opt.address, address, cfg->opt.max_ttl, cfg->opt.packetlen);
	return 0;
}

void destroy_route(t_route* route)
{
	int i;
	for (i = 0; i < route->maxlen; i++)
	{
		if (route->hops[i].probes)
			free(route->hops[i].probes);
		else
			break ;
	}
	free(route->hops);
}

int send_probes(t_config* cfg, t_route* route)
{
	t_hop* current_hop = route->hops + (route->last_ttl - cfg->opt.first_ttl);
	int total = 0;

	while (total < cfg->opt.squeries)
	{
		if (current_hop->probes == NULL)
		{
			/* Allocate a new array of probes for the next hop */
			current_hop->probes = ft_calloc(cfg->opt.nqueries, sizeof(t_probe));
			if (!current_hop->probes)
				return -1;

			/* Set ttl field for the future packets */
			if (setsockopt(cfg->sockfd, IPPROTO_IP, IP_TTL, &route->last_ttl, sizeof(uint8_t)) == -1)
				return -1;
			route->last_ttl++;
		}

		/*
		** Set destination port
		** we don't want the host to process the probe
		** so we send it on an "unlikely" port
		*/
		if (cfg->host.ai_family == AF_INET)
			((struct sockaddr_in*)cfg->host.ai_addr)->sin_port = htons(cfg->opt.port);
		else
			((struct sockaddr_in6*)cfg->host.ai_addr)->sin6_port = htons(cfg->opt.port);

		init_probe(current_hop->probes + current_hop->nb_sent, cfg->opt.port); 
		if (sendto(cfg->sockfd, NULL, 0, 0, cfg->host.ai_addr, cfg->host.ai_addrlen) >= 0)
			cfg->opt.port++; // increased for each probe, useful to match them with responses
		else
			return -1;

		total++;
		current_hop->nb_sent++;

		/* Increase ttl after sending all probes for current hop */
		if (current_hop->nb_sent == cfg->opt.nqueries)
			current_hop++;
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
		{
			log_error("timeout");
			return 1; // no replies after timeout
		}

		/* Receive ICMP message */
		char buf[100];
		struct sockaddr_storage address;
		socklen_t addrlen = sizeof(address);
		ssize_t bytes = recvfrom(cfg->icmp_sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&address, &addrlen);
		if (bytes == -1)
			return -1;

		/*
		** Match one of the probes sent to the response
		** UDP destination port is used to identify probes
		**
		** original packet can be found after IP and ICMP headers
		** UDP header is placed after the IP header in the original packet
		*/
		struct icmphdr* icmp = (struct icmphdr*)(buf + sizeof(struct iphdr));

		// compare checksums
		// check icmp type
		// check packet is ours

		uint8_t ttl;
		uint16_t id;
		extract_probe_info((const char*)(icmp + 1), &ttl, &id);

		t_hop* hop = route->hops + ttl - cfg->opt.first_ttl;
		hop->nb_recvd++;
		if (save_gateway(hop, (struct sockaddr*)&address, id) == -1)
			return -1;

		count++;
	}

	return 0;	
}

void log_route(t_config* cfg, t_route* route)
{
	t_hop* last_hop = route->hops + route->len;

	while (last_hop->nb_recvd == cfg->opt.nqueries)
	{
		route->len++;

		printf(" %d  ", route->len);

		t_probe* p;
		const char* gateway = NULL;
		for (p = last_hop->probes; p - last_hop->probes < last_hop->nb_sent; p++)
		{
			if (!gateway || ft_strncmp(gateway, p->address, INET6_ADDRSTRLEN) != 0)
				printf("%s (%s) ", p->hostname, p->address);
			printf(" %.3f ms ", get_duration_ms(&p->time_sent, &p->time_recvd));

			gateway = p->address;
		}
		printf("\n");

		last_hop++;
	}
}
