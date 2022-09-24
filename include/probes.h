/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   probes.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/21 13:19:09 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/23 19:08:53 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _PROBES_H_
# define _PROBES_H_

# include <arpa/inet.h>
# include <stdint.h>
# include <limits.h>
# include <time.h>

#include "ft_traceroute.h"

typedef struct s_probe
{
	/* Set after sendto() */
	uint16_t id;
	struct timeval time_sent;

	/* Set after recvfrom() */
	char hostname[HOST_NAME_MAX];
	char address[INET6_ADDRSTRLEN];
	struct timeval time_recvd;
} t_probe;

typedef struct s_hop
{
	t_probe* probes;
	unsigned int nb_sent;
	unsigned int nb_recvd;
} t_hop;

void init_probe(t_probe* ptr, uint16_t id);
void extract_probe_info(const char* packetbuf, uint8_t* ttl, uint16_t* id);
int save_gateway(t_hop* hop, const struct sockaddr* addr, uint16_t id);

#endif
