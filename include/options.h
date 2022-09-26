/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/19 19:05:47 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/26 19:11:09 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _OPTIONS_H_
# define _OPTIONS_H_

# include <stdint.h>
# include <netdb.h>

# include "libft.h"

# define N_OPTIONS_SUPPORTED	11
# define ERRMSG_MAXLEN			64
# define PACKETLEN_MIN			28
# define PACKETLEN_MAX			65000

typedef struct s_waittime
{
	double max;
	double here;
	double near;
} t_waittime;

typedef struct s_cmdline_args
{
	bool dns_enabled;
	uint8_t family;
	uint8_t protocol;
	uint8_t socktype;

	uint8_t first_ttl;	// -f
	uint8_t max_ttl;	// -m
	uint8_t squeries;	// -N
	uint16_t port;		// -p
	uint8_t nqueries;	// -q
	t_waittime timeout;	// -w
	
	char* address;
	uint16_t packetlen;
} t_cmdline_args;

t_cmdline_args init_options_struct();
int parse_arguments(int argc, char** argv, t_cmdline_args* opt);
int parse_waittime(char* buffer, t_waittime* result, char* errmsg);
int print_usage();

#endif
