/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packets.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/21 13:19:09 by mamartin          #+#    #+#             */
/*   Updated: 2022/10/04 05:27:27 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _PROBES_H_
# define _PROBES_H_

# include <arpa/inet.h>
# include <stdint.h>
# include <limits.h>
# include <time.h>
# include <stdbool.h>

struct s_config;
typedef struct s_config t_config;

typedef enum e_probe_state
{
	WAITING_REPLY,
	WAITING_NAME_INFO,
	TIMED_OUT,
	SUCCESS
} t_probe_state;

typedef struct s_probe
{
	t_probe_state status;
	pthread_mutex_t mut;

	/* Set after recvfrom() */
	char hostname[HOST_NAME_MAX];
	char address[INET6_ADDRSTRLEN];
	float rtt;

	/* Set after sendto() */
	struct timeval time_sent;
	uint16_t id;
} t_probe;

typedef struct s_hop
{
	t_probe* probes;
	unsigned int nb_sent;
	unsigned int nb_recvd;
	bool is_destination;
} t_hop;

int init_probe(t_probe* ptr, uint16_t id);

uint8_t validate_packet(char* payload, size_t len, t_config* cfg);
uint16_t extract_probe_id(char* payload, struct sockaddr_storage* addr);
uint16_t compute_checksum(uint16_t* data, size_t bytes);
int compare_checksums(uint16_t* data, size_t bytes, uint16_t* cs);

#endif
