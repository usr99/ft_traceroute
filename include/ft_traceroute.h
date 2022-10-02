/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/18 10:17:35 by mamartin          #+#    #+#             */
/*   Updated: 2022/10/02 09:37:36 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _FT_TRACEROUTE_H_
# define _FT_TRACEROUTE_H_

# include "dns.h"
# include "options.h"
# include "packets.h"
# include "libft.h"

typedef struct s_config
{
	t_cmdline_args opt;
	struct sockaddr_storage host;
	t_rdns_slot* dns_threads;
	int sockfd;
	int icmp_sockfd;
} t_config;

typedef struct s_route
{
	t_hop* hops;
	int current_ttl;
	uint8_t len;
	uint8_t maxlen;
} t_route;

/* main.c */
int setup_tracerouting(int argc, char** argv, t_config* cfg);
int init_route(t_config* cfg, t_route* route);
void destroy_route(t_route* route);

/* tracerouting.c */
int send_probes(t_config* cfg, t_route* route, unsigned int *nprobes);
int recv_response(t_config* cfg, t_route* route, unsigned int *nprobes);
int check_timeout(t_config* cfg, t_route* route);
bool browse_route(t_config* cfg, t_route* route);

/* utils.c */
int log_error(const char* message);
float get_duration_from_now(struct timeval* from);
float get_duration_ms(struct timeval* from, struct timeval* to);
int addr_to_text(const struct sockaddr_storage* addr, char* buffer);
bool compare_ipv6_addresses(const struct in6_addr* rhs, const struct sockaddr_in6* lhs);

/* debug.c */
void debug_options(t_cmdline_args* opt);
void debug_route(const t_route* route, const t_config* cfg);

#endif
