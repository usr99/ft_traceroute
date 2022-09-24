/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/18 10:17:35 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/23 22:10:04 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _FT_TRACEROUTE_H_
# define _FT_TRACEROUTE_H_

# include "options.h"
# include "probes.h"
# include "libft.h"

typedef struct s_config
{
	t_cmdline_args opt;
	struct sockaddr_storage host;
	int sockfd;
	int icmp_sockfd;
} t_config;

typedef struct s_route
{
	t_hop* hops;
	uint8_t last_ttl;
	uint8_t len;
	uint8_t maxlen;
} t_route;

/* ft_traceroute.c */
int setup_tracerouting(int argc, char** argv, t_config* cfg);
int init_route(t_config* cfg, t_route* route);
void destroy_route(t_route* route);

int send_probes(t_config* cfg, t_route* route);
int recv_response(t_config* cfg, t_route* route);
void log_route(t_config* cfg, t_route* route);

/* utils.c */
int log_error(const char* message);
float get_duration_from_now(struct timeval* from);
float get_duration_ms(struct timeval* from, struct timeval* to);

/* debug.c */
void debug_options(t_cmdline_args* opt);
void debug_route(const t_route* route, const t_config* cfg);

#endif
