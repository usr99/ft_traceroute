/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/19 19:05:47 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/20 17:20:26 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _OPTIONS_H_
# define _OPTIONS_H_

# include <stdint.h>
# include <netdb.h>

# define N_OPTIONS_SUPPORTED	9

# define DEFAULT_PROTOCOL		IPPROTO_UDP
# define DEFAULT_SOCKTYPE		SOCK_DGRAM
# define DEFAULT_FIRST_TTL		1
# define DEFAULT_MAX_TTL		30
# define DEFAULT_SQUERIES		16
# define DEFAULT_PORT			33434
# define DEFAULT_IMCP_SEQ		1
# define DEFAULT_WAITTIME		5.0f
# define DEFAULT_NQUERIES		3
# define DEFAULT_PACKETLEN		60

# define PACKETLEN_MIN			28
# define PACKETLEN_MAX			65000

typedef struct s_cmdline_args
{
	bool forceIPv6;
	bool dns_enabled;
	uint8_t protocol;
	uint8_t socktype;

	uint8_t first_ttl;	// -f
	uint8_t max_ttl;	// -m
	uint8_t squeries;	// -N
	uint16_t port;		// -p
	uint16_t icmp_seq;	// -p
	float waittime;		// -w
	uint8_t nqueries;	// -q
	
	char* address;
	uint16_t packetlen;

	// interface -i
	// tos -t
	// source_addr -s
} t_cmdline_args;

void init_options_struct(t_cmdline_args* opt);
t_cmdline_args parse_arguments(int argc, char** argv);
void print_usage();
void debug_options(t_cmdline_args* opt);

#endif
