/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/18 10:17:35 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/18 11:06:35 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _FT_TRACEROUTE_H_
# define _FT_TRACEROUTE_H_

# include <stdint.h>

# define N_OPTIONS_SUPPORTED 9

enum e_options_flags
{
	OPT_FORCE_IPV6			= 1, // -6
	OPT_USE_ICMP			= 2, // -I
	OPT_NO_DNS_RESOLUTION	= 4	 // -n
};

typedef struct s_tr_options
{
	uint8_t bitmask;

	uint8_t first_ttl;	// -f
	uint8_t max_ttl;	// -m
	uint8_t squeries;	// -N
	uint16_t port;		// -p
	uint32_t waittime;	// -w
	uint8_t nqueries;	// -q
	
	// interface -i
	// tos -t
	// source_addr -s
} t_tr_options;

#endif
