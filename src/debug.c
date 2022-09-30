/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/23 17:37:32 by mamartin          #+#    #+#             */
/*   Updated: 2022/10/01 00:45:05 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include "ft_traceroute.h"

void debug_options(t_cmdline_args* opt)
{
	printf("-6 = %d\n", opt->family == AF_INET6);
	printf("-I = %d\n", opt->protocol == IPPROTO_ICMP);
	printf("-T = %d\n", opt->protocol == IPPROTO_TCP);
	printf("-n = %d\n", opt->dns_enabled);

	printf("-f = %d\n", opt->first_ttl);
	printf("-m = %d\n", opt->max_ttl);
	printf("-N = %d\n", opt->squeries);
	printf("-p UDP = %d\n", opt->port);
	printf("-w = %f/%f/%f\n", opt->timeout.max, opt->timeout.here, opt->timeout.near);
	printf("-q = %d\n", opt->nqueries);

	printf("address = %s\n", opt->address);
	printf("packetlen = %d\n", opt->packetlen);
}

void debug_route(const t_route* route, const t_config* cfg)
{
	int i;
	unsigned int j;

	for (i = 0; i < route->current_ttl; i++)
	{
		printf("%d", i + cfg->opt.first_ttl);
		for (j = 0; j < route->hops[i].nb_sent; j++)
		{
			printf("\tid=%d", route->hops[i].probes[j].id);
			if (route->hops[i].probes[j].status == SUCCESS)
			{
				printf(" from %s (%s) %.3f\n",
					route->hops[i].probes[j].hostname,
					route->hops[i].probes[j].address,
					route->hops[i].probes[j].rtt
				);
			}
			else if (route->hops[i].probes[j].status == TIMED_OUT)
				printf(" TIMEOUT\n");
		}
		printf("\n");
	}
}
