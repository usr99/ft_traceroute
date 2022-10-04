/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/18 10:13:43 by mamartin          #+#    #+#             */
/*   Updated: 2022/10/04 05:34:59 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pthread.h>
#include <linux/icmp.h>
#include <netinet/icmp6.h>
#include <netinet/ip6.h>
#include <stdio.h>

#include "ft_traceroute.h"
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

	/* count of probes currently waiting for reply */
	unsigned int nprobes = 0; // must never exceed <squeries> (set with -N, default 16)

	bool host_found = false;
	while (!host_found)
	{
		if (send_probes(&config, &route, &nprobes) != 0)
		{
			printf("Failure occured when trying to send probes\n");
			break; // fatal error
		}

		if (recv_response(&config, &route, &nprobes) != 0)
		{
			printf("Failure occured when trying to receive responses\n");
			break; // fatal error
		}
		
		host_found = browse_route(&config, &route);
	}

	close(config.sockfd);
	close(config.icmp_sockfd);
	if (config.opt.dns_enabled)
	{
		int i;
		for (i = 0; i < config.opt.squeries; i++)
		{
			if (config.dns_threads[i].in_use)
				pthread_join(config.dns_threads[i].id, NULL);
		}
		free(config.dns_threads);
	}
	destroy_route(&route);
	return 0;
}

int setup_tracerouting(int argc, char** argv, t_config* cfg)
{
	cfg->opt = init_options_struct();
	if (parse_arguments(argc, argv, &cfg->opt) == -1)
		return -1;

	if (getuid() != 0)
		return log_error("user must have root privileges !");

	if (resolve_hostname(&cfg->host, &cfg->opt) == -1)
		return -1;

	if ((cfg->sockfd = socket(cfg->opt.family, cfg->opt.socktype, cfg->opt.protocol)) == -1)
		return log_error("failed to create UDP socket");

	int protocol = (cfg->host.ss_family == AF_INET) ? IPPROTO_ICMP : IPPROTO_ICMPV6;
	if ((cfg->icmp_sockfd = socket(cfg->host.ss_family, SOCK_RAW | SOCK_NONBLOCK, protocol)) == -1)
	{
		close(cfg->sockfd);
		return log_error("failed to create ICMP socket");
	}

	/* Only accepts "Destination Unreachable" and "Time Exceeded" */
	struct icmp_filter filterv4 = { .data = ~(1 << ICMP_DEST_UNREACH | 1 << ICMP_TIME_EXCEEDED) };
	struct icmp6_filter filterv6;
	
	void* filter_ptr;
	size_t filter_size;
	if (cfg->host.ss_family == AF_INET)
	{
		filter_size = sizeof(struct icmp_filter);
		filter_ptr = &filterv4;
	}
	else
	{
		ICMP6_FILTER_SETBLOCKALL(&filterv6);
		ICMP6_FILTER_SETPASS(ICMP6_DST_UNREACH, &filterv6);
		ICMP6_FILTER_SETPASS(ICMP6_TIME_EXCEEDED, &filterv6);

		filter_size = sizeof(struct icmp6_filter);
		filter_ptr = &filterv6;
	}

	if (setsockopt(cfg->icmp_sockfd, protocol, ICMP_FILTER, filter_ptr, filter_size) != 0)
	{
		close(cfg->sockfd);
		close(cfg->icmp_sockfd);
		return log_error("failed to set ICMP filter");
	}

	if (cfg->opt.dns_enabled)
	{
		cfg->dns_threads = ft_calloc(sizeof(t_rdns_slot), cfg->opt.squeries);
		if (!cfg->dns_threads)
		{
			close(cfg->sockfd);
			close(cfg->icmp_sockfd);
			return log_error("Out of memory");
		}
	}

	return 0;
}

int init_route(t_config* cfg, t_route* route)
{
	char address[INET6_ADDRSTRLEN];
	if (addr_to_text(&cfg->host, address) == -1)
		return -1;

	const char* ret;
	if (cfg->host.ss_family == AF_INET)
		ret = inet_ntop(cfg->opt.family, &((struct sockaddr_in*)&cfg->host)->sin_addr.s_addr, address, INET6_ADDRSTRLEN);
	else
		ret = inet_ntop(cfg->opt.family, ((struct sockaddr_in6*)&cfg->host)->sin6_addr.in6_u.u6_addr32, address, INET6_ADDRSTRLEN);
	if (!ret)
		return -1;

	route->len = 0;
	route->current_ttl = cfg->opt.first_ttl;
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
	unsigned int j;

	for (i = 0; i < route->maxlen; i++)
	{
		if (route->hops[i].probes)
		{
			for (j = 0; j < route->hops[i].nb_sent; j++)
				pthread_mutex_destroy(&route->hops[i].probes[j].mut);
			free(route->hops[i].probes);
		}
		else
			break ;
	}
	free(route->hops);
}
