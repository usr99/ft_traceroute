/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packets.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/21 13:19:09 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/24 17:02:30 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _PROBES_H_
# define _PROBES_H_

# include <arpa/inet.h>
# include <stdint.h>
# include <limits.h>
# include <time.h>

struct s_config;
typedef struct s_config t_config;

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

int process_response(char* payload, size_t len, t_hop* hops, t_config* cfg);
int validate_packet(char* payload, size_t len, t_config* cfg);
void parse_packet(char* payload, struct sockaddr_in* addr, uint8_t* ttl, uint16_t* id);
uint16_t compute_checksum(uint16_t* data, size_t bytes);
int compare_checksums(uint16_t* data, size_t bytes, uint16_t* cs);

#endif