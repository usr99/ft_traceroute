/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/19 18:47:03 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/20 17:20:55 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>

#include "ft_traceroute.h"
#include "options.h"
#include "libft.h"

void init_options_struct(t_cmdline_args* opt)
{
	opt->forceIPv6 = false;
	opt->dns_enabled = true;
	opt->protocol = DEFAULT_PROTOCOL;
	opt->protocol = DEFAULT_SOCKTYPE;

	opt->first_ttl = DEFAULT_FIRST_TTL;
	opt->max_ttl = DEFAULT_MAX_TTL;
	opt->squeries = DEFAULT_SQUERIES;
	opt->port = DEFAULT_PORT;
	opt->icmp_seq = DEFAULT_IMCP_SEQ;
	opt->waittime = DEFAULT_WAITTIME;
	opt->nqueries = DEFAULT_NQUERIES;

	opt->address = NULL;
	opt->packetlen = DEFAULT_PACKETLEN;
}

t_cmdline_args parse_arguments(int argc, char** argv)
{
	static t_expected_opts valid_options[N_OPTIONS_SUPPORTED] = {
		{ .name = '6', .has_param = false },
		{ .name = 'I', .has_param = false },
		{ .name = 'T', .has_param = false },
		{ .name = 'n', .has_param = false },
		{ .name = 'f', .has_param = true, .paramtype = PARAM_T_INT32 },
		{ .name = 'm', .has_param = true, .paramtype = PARAM_T_INT32 },
		{ .name = 'N', .has_param = true, .paramtype = PARAM_T_INT32 },
		{ .name = 'q', .has_param = true, .paramtype = PARAM_T_INT32 },
		{ .name = 'p', .has_param = true, .paramtype = PARAM_T_INT32 },
		{ .name = 'w', .has_param = true, .paramtype = PARAM_T_INT32 }
	};

	t_cmdline_args options;
	t_argument arg;
	int ret;
	int val;
	int nparameters = 0;

	init_options_struct(&options);
	while ((ret = ft_getarg(argc, argv, valid_options, N_OPTIONS_SUPPORTED, &arg)) == 0)
	{
		switch (arg.type)
		{
			case ARG_T_OPTION:
				switch (arg.info.opt.name)
				{
					case '6':
						options.forceIPv6 = true;
						break;
					case 'I':
						options.protocol = IPPROTO_ICMP;
						options.socktype = SOCK_RAW;
						break;
					case 'T':
						options.protocol = IPPROTO_TCP;
						options.socktype = SOCK_STREAM;
						break;
					case 'n':
						options.dns_enabled = false;
						break;
					case 'f':
						options.first_ttl = *(uint8_t*)arg.info.opt.value;
						break;
					case 'm':
						val = *(int*)arg.info.opt.value;
						if (val < 0 || val > 255)	
							exit_error("max hops cannot be more than 255");
						options.max_ttl = (uint8_t)val;
						break;
					case 'N':
						options.squeries = *(uint8_t*)arg.info.opt.value;
						break;
					case 'q':
						options.nqueries = *(uint8_t*)arg.info.opt.value;
						if (options.nqueries == 0 || options.nqueries > 10)
							exit_error("no more than 10 probes per hop");
						break;
					case 'p':
						options.port = *(uint16_t*)arg.info.opt.value;
						break;
					case 'w':
						options.waittime = (float)*(uint32_t*)arg.info.opt.value;
						break;
					default: // should never happen
						break;
				}
				if (arg.info.opt.value)
					free(arg.info.opt.value);
				break ;
			case ARG_T_PARAMETER:
				nparameters++;
				if (nparameters == 1)
					options.address = (char*)arg.info.param.value;
				else if (nparameters == 2)
				{
					val = ft_atoi((char*)arg.info.param.value);
					if (val < PACKETLEN_MIN)
						val = PACKETLEN_MIN;
					else if (val > PACKETLEN_MAX)
						exit_error("too big packetlen specified");
					options.packetlen = val;
				}
				else
					exit_error("too much arguments specified");
				break ;
			case ARG_T_ERROR:
				switch (arg.info.err.type)
				{
					case ERR_BAD_OPTION:
						exit_error("Bad option");
						break;
					case ERR_MISSING_PARAM:
						exit_error("Option requires an argument");
						break;
					case ERR_BAD_PARAM_TYPE:
						exit_error("Cannot handle option with arg");
						break;
				}
				break ;
		}
	}

	if (ret == -2) // code to indicate memory allocation failure
		exit_error("Out of memory");

	if (!options.address)
		(argc != 1) ? exit_error("Specify \"host\" missing argument.") : print_usage(argv[0]);
	return options;
}

void print_usage(const char* program_name)
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
		"\t\t -w MAX,HERE,NEAR  --wait=MAX,HERE,NEAR\n"
		"\t\tWait for a probe no more than HERE (default 3)\n"
		"\t\ttimes longer than a response from the same hop,\n"
		"\t\tor no more than NEAR (default 10) times than some\n"
		"\t\tnext hop, or MAX (default 5.0) seconds (float\n"
		"\t\tpoint values allowed too)\n"
		" -q nqueries\tSet the number of probes per each hop. Default is 3\n",
		program_name
	);
	exit(EXIT_SUCCESS);
}

void debug_options(t_cmdline_args* opt)
{
	printf("-6 = %d\n", opt->forceIPv6);
	printf("-I = %d\n", opt->protocol == IPPROTO_ICMP);
	printf("-T = %d\n", opt->protocol == IPPROTO_TCP);
	printf("-n = %d\n", opt->dns_enabled);

	printf("-f = %d\n", opt->first_ttl);
	printf("-m = %d\n", opt->max_ttl);
	printf("-N = %d\n", opt->squeries);
	printf("-p UDP = %d\n", opt->port);
	printf("-p ICMP = %d\n", opt->icmp_seq);
	printf("-w = %f\n", opt->waittime);
	printf("-q = %d\n", opt->nqueries);

	printf("address = %s\n", opt->address);
	printf("packetlen = %d\n", opt->packetlen);
}

