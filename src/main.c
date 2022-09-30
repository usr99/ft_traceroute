/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/18 10:13:43 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/29 16:31:12 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <linux/icmp.h>
#include <stdio.h>

#include "ft_traceroute.h"

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
			printf("fatal error\n");
			break; // fatal error
		}
		if (recv_response(&config, &route, &nprobes) != 0)
		{
			printf("fatal error\n");
			break; // fatal error
		}
		
		host_found = browse_route(&config, &route);
	}

	// debug_route(&route, &config);

	close(config.sockfd);
	close(config.icmp_sockfd);
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

	if ((cfg->icmp_sockfd = socket(AF_INET, SOCK_RAW | SOCK_NONBLOCK, IPPROTO_ICMP)) == -1)
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
	if (addr_to_text(&cfg->host, address) == -1)
		return -1;

	const char* ret;
	if (cfg->host.ss_family == AF_INET)
		ret = inet_ntop(cfg->opt.family, &((struct sockaddr_in*)&cfg->host)->sin_addr.s_addr, address, INET6_ADDRSTRLEN);
	else
		ret = inet_ntop(cfg->opt.family, ((struct sockaddr_in6*)&cfg->host)->sin6_addr.__in6_u.__u6_addr32, address, INET6_ADDRSTRLEN);
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
	for (i = 0; i < route->current_ttl; i++)
	{
		if (route->hops[i].probes)
			free(route->hops[i].probes);
		else
			break ;
	}
	free(route->hops);
}
