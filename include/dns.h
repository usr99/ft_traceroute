/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dns.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/02 03:31:57 by mamartin          #+#    #+#             */
/*   Updated: 2022/10/02 23:34:55 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _DNS_H_
# define _DNS_H_

#include <stdbool.h>
#include <pthread.h>

typedef struct s_config t_config;
typedef struct s_cmdline_args t_cmdline_args;
typedef struct s_probe t_probe;

typedef struct s_rdns_slot
{
	pthread_t id;
	bool in_use;
} t_rdns_slot;

typedef struct s_rdns_args
{
	struct sockaddr_storage* addr;
	t_probe* gateway_info;
	t_rdns_slot *slot;
} t_rdns_args;

/* name_resolution.c */
int resolve_hostname(struct sockaddr_storage* host, t_cmdline_args* opt);
int reverse_dns_lookup(struct sockaddr_storage* addr, t_probe* gateway, t_config* cfg);
void* thread_routine(void* p);

#endif
