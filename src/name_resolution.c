/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   name_resolution.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/20 20:19:46 by mamartin          #+#    #+#             */
/*   Updated: 2022/10/02 23:34:42 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pthread.h>
#include <stdio.h>

#include "ft_traceroute.h"
#include "dns.h"
#include "libft.h"

int resolve_hostname(struct sockaddr_storage* host, t_cmdline_args* opt)
{
	struct addrinfo *results;
	struct addrinfo hint = {
		.ai_family = opt->family,
		.ai_socktype = opt->socktype,
		.ai_protocol = opt->protocol,
		.ai_flags = AI_ADDRCONFIG
	};

	int code;
	if ((code = getaddrinfo(opt->address, NULL, &hint, &results)) != 0)
	{
		dprintf(STDERR_FILENO, "%s: ", opt->address);
		return log_error(gai_strerror(code));
	}
		
	ft_memcpy(host, results->ai_addr, results->ai_addrlen);
	freeaddrinfo(results);
	return 0;
}

int reverse_dns_lookup(struct sockaddr_storage* addr, t_probe* gateway, t_config* cfg)
{
	t_rdns_args *p;
	int i;

	p = malloc(sizeof(t_rdns_args));
	if (!p)
		return -1;
	p->addr = malloc(sizeof(struct sockaddr_storage));
	if (!p->addr)
	{
		free(p);
		return -1;
	}

	ft_memcpy(p->addr, addr, sizeof(struct sockaddr_storage));
	p->addr->ss_family = cfg->opt.family;
	p->gateway_info = gateway;
	for (i = 0; i < cfg->opt.squeries && cfg->dns_threads[i].in_use; i++);
	p->slot = cfg->dns_threads + i;
	p->slot->in_use = true;

	if (pthread_create(&p->slot->id, NULL, &thread_routine, p) != 0)
		return -1;
	return 0;
}

void* thread_routine(void* p)
{
	t_rdns_args* params = (t_rdns_args*)p;

	if (getnameinfo((struct sockaddr*)params->addr, sizeof(struct sockaddr_storage), params->gateway_info->hostname, HOST_NAME_MAX, NULL, 0, 0) != 0)
		params->gateway_info->hostname[0] = '\0';
	params->gateway_info->status = SUCCESS;
	params->slot->in_use = false;
	
	pthread_detach(pthread_self());
	
	free(params->addr);
	free(params);
	return NULL; // unused
}
