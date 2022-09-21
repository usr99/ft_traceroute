/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   probes.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/21 13:19:09 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/21 19:34:29 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _PROBES_H_
# define _PROBES_H_

# include <arpa/inet.h>
# include <stdint.h>
# include <limits.h>
# include <time.h>

# include "libft.h"

typedef struct s_ttl_settings
{
	uint8_t ttl;
	int nb_sent;
	int nb_recvd;
	t_list* probes; // t_probe_packet
} t_ttl_settings;

typedef struct s_probe_packet
{
	uint16_t id;
	struct timeval time_sent;
	t_list* replies; // t_gateway_response
} t_probe_packet;

typedef struct s_gateway_response
{
	char address[INET6_ADDRSTRLEN];
	char name[HOST_NAME_MAX];
	struct timeval time;
} t_gateway_response;

t_ttl_settings* push_new_ttl_setting(t_list** queries, uint8_t ttl);
t_probe_packet* push_new_probe_packet(t_list** probes, uint16_t id);
t_gateway_response* push_new_response(t_list** gateways, struct sockaddr* addr, struct timeval timestamp);
t_probe_packet* get_probe_from_id(t_list* queries, uint16_t id);

void debug_queries(t_list* queries);

#endif
