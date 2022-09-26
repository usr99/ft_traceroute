/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/19 18:47:03 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/26 18:00:46 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>

#include "ft_traceroute.h"
#include "options.h"

void init_options_struct(t_cmdline_args* opt)
{
	opt->dns_enabled = true;
	opt->family = DEFAULT_FAMILY;
	opt->protocol = DEFAULT_PROTOCOL;
	opt->socktype = DEFAULT_SOCKTYPE;

	opt->first_ttl = DEFAULT_FIRST_TTL;
	opt->max_ttl = DEFAULT_MAX_TTL;
	opt->squeries = DEFAULT_SQUERIES;
	opt->port = DEFAULT_PORT;
	opt->icmpseq = DEFAULT_IMCP_SEQ;
	opt->waittime = DEFAULT_WAITTIME;
	opt->nqueries = DEFAULT_NQUERIES;

	opt->address = NULL;
	opt->packetlen = DEFAULT_PACKETLEN;
}

int parse_arguments(int argc, char** argv, t_cmdline_args* opt)
{
	static t_expected_opts valid_options[N_OPTIONS_SUPPORTED] = {
		{ .name = '6', .has_param = false },
		{ .name = 'I', .has_param = false },
		{ .name = 'T', .has_param = false },
		{ .name = 'n', .has_param = false },
		{ .name = 'h', .has_param = false },
		{ .name = 'f', .has_param = true, .paramtype = PARAM_T_INT64 },
		{ .name = 'm', .has_param = true, .paramtype = PARAM_T_INT64 },
		{ .name = 'N', .has_param = true, .paramtype = PARAM_T_INT64 },
		{ .name = 'q', .has_param = true, .paramtype = PARAM_T_INT64 },
		{ .name = 'p', .has_param = true, .paramtype = PARAM_T_INT64 },
		{ .name = 'w', .has_param = true, .paramtype = PARAM_T_FLOAT64 }
	};

	t_argument arg;
	const char* errmsg = NULL;
	int ret;
	int val;
	int nparameters = 0;

	init_options_struct(opt);
	while ((ret = ft_getarg(argc, argv, valid_options, N_OPTIONS_SUPPORTED, &arg)) == 0)
	{
		switch (arg.type)
		{
			case ARG_T_OPTION:
				switch (arg.name)
				{
					case '6':
						opt->family = AF_INET6;
						break;
					case 'I':
						opt->protocol = IPPROTO_ICMP;
						opt->socktype = SOCK_RAW;
						break;
					case 'T':
						opt->protocol = IPPROTO_TCP;
						opt->socktype = SOCK_STREAM;
						break;
					case 'n':
						opt->dns_enabled = false;
						break;
					case 'h':
						print_usage(argv[0]);
						break;						
					case 'f':
						if (*(int*)arg.value <= 0)
							errmsg = "first hop out of range";
						else
							opt->first_ttl = *(uint8_t*)arg.value;
						break;
					case 'm':
						if (*(int*)arg.value < 0 || *(int*)arg.value > 255)
							errmsg = "max hops cannot be more than 255";
						else
							opt->max_ttl = *(uint8_t*)arg.value;
						break;
					case 'N':
						opt->squeries = *(uint8_t*)arg.value;
						break;
					case 'q':
						opt->nqueries = *(uint8_t*)arg.value;
						if (opt->nqueries == 0 || opt->nqueries > 10)
							errmsg = "no more than 10 probes per hop";
						break;
					case 'p':
						opt->port = *(uint16_t*)arg.value;
						break;
					case 'w':
						opt->waittime = *(double*)arg.value;
						if (opt->waittime < 0.)
							errmsg = "wait time cannot be negative";
						break;
					default: // should never happen
						break;
				}
				if (arg.value)
					free(arg.value);
				if (errmsg)
					return log_error(errmsg);
				break ;
			case ARG_T_PARAMETER:
				nparameters++;
				if (nparameters == 1)
					opt->address = (char*)arg.value;
				else if (nparameters == 2)
				{
					if (ft_strtol((char*)arg.value, (long*)&val) == -1)
						return log_error("packetlen expect an integer as argument");
					if (val < PACKETLEN_MIN)
						val = PACKETLEN_MIN;
					else if (val > PACKETLEN_MAX)
						return log_error("too big packetlen specified");
					opt->packetlen = val;
				}
				else
					return log_error("too much arguments specified");
				break ;
			case ARG_T_ERROR:
				switch (arg.errtype)
				{
					case ERR_BAD_OPTION:
						return log_error("Bad option");
					case ERR_MISSING_PARAM:
						return log_error("Option requires an argument");
					case ERR_BAD_PARAM_TYPE:
						return log_error("Cannot handle option with arg");
					default: // should never happen
						break;
				}
				break ;
		}
	}

	if (ret == -2) // code to indicate memory allocation failure
		return log_error("Out of memory");

	if (opt->first_ttl > opt->max_ttl)
		return log_error("first hop out of range");
	if (opt->squeries < 1)
		opt->squeries = 1;

	if (!opt->address)
		return (argc != 1) ? log_error("Specify \"host\" missing argument.") : print_usage(argv[0]);
	return 0;
}

int print_usage(const char* program_name)
{
	dprintf(STDERR_FILENO,
		"Usage:\n"
		" %s [ -6In ][ -f first_ttl ] [ -m max_ttl ] [ -N squeries ] [ -p port ]"
		"[ -w MAX,HERE,NEAR ] [ -q nqueries ] host [ packetlen ]\n\n"
		"Options:\n"
		" -6 \t\tUse IPv6\n"
		" -f first_ttl\tStart from the first_ttl hop (instead of 1)\n"
		" -I \t\tUse ICMP ECHO for tracerouting\n"
		" -m max_ttl\tSet the max number of hops (max TTL to be reached). Default is 30\n"
		" -N squeries\tSet the number of probes to be tried simultaneously (default is 16)\n"
		" -n \t\tDo not resolve IP addresses to their domain names\n"
		" -p port\tSet the destination port to use. It is either initial udp port value for \"default\"\n"
		"\t\tmethod(incremented by each probe, default is 33434),\n"
		"\t\tor initial seq for \"icmp\"(incremented as well, default from 1)\n"
		" -w MAX\t\tWait for a probe no more than MAX seconds (default 5.0)\n"
		" -q nqueries\tSet the number of probes per each hop. Default is 3\n",
		program_name
	);
	exit(EXIT_SUCCESS);
}
