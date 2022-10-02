/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/19 18:47:03 by mamartin          #+#    #+#             */
/*   Updated: 2022/10/03 00:00:56 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>

#include "ft_traceroute.h"
#include "options.h"

t_cmdline_args init_options_struct()
{
	t_cmdline_args result = {
		.family = AF_INET, .protocol = IPPROTO_UDP, .socktype = SOCK_DGRAM,
		.first_ttl = 1, .max_ttl = 30,
		.squeries = 16, .nqueries = 3,
		.port = 33434, .timeout = { 5.0f, 3.0f, 10.0f },
		.address = NULL, .packetlen = 60, .dns_enabled = true
	};
	return result;
}

int parse_arguments(int argc, char** argv, t_cmdline_args* opt)
{
	static t_expected_opts valid_options[N_OPTIONS_SUPPORTED] = {
		{ .name = '6', .has_param = false },
		{ .name = 'n', .has_param = false },
		{ .name = 'h', .has_param = false },
		{ .name = 'f', .has_param = true, .paramtype = PARAM_T_INT64 },
		{ .name = 'm', .has_param = true, .paramtype = PARAM_T_INT64 },
		{ .name = 'N', .has_param = true, .paramtype = PARAM_T_INT64 },
		{ .name = 'q', .has_param = true, .paramtype = PARAM_T_INT64 },
		{ .name = 'p', .has_param = true, .paramtype = PARAM_T_INT64 },
		{ .name = 'w', .has_param = true, .paramtype = PARAM_T_STRING }
	};

	t_argument arg;
	char errmsg[ERRMSG_MAXLEN] = { 0 };
	int ret;
	int val;
	int nparameters = 0;

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
							ft_strlcpy(errmsg, "first hop out of range", ERRMSG_MAXLEN);
						else
							opt->first_ttl = *(uint8_t*)arg.value;
						break;
					case 'm':
						if (*(int*)arg.value < 0 || *(int*)arg.value > 255)
							ft_strlcpy(errmsg, "max hops cannot be more than 255", ERRMSG_MAXLEN);
						else
							opt->max_ttl = *(uint8_t*)arg.value;
						break;
					case 'N':
						opt->squeries = *(uint8_t*)arg.value;
						break;
					case 'q':
						opt->nqueries = *(uint8_t*)arg.value;
						if (opt->nqueries == 0 || opt->nqueries > 10)
							ft_strlcpy(errmsg, "no more than 10 probes per hop", ERRMSG_MAXLEN);
						break;
					case 'p':
						opt->port = *(uint16_t*)arg.value;
						break;
					case 'w':
						parse_waittime(arg.value, &opt->timeout, errmsg);
						break;
					default: // should never happen
						break;
				}
				if (arg.value)
					free(arg.value);
				break ;
			case ARG_T_PARAMETER:
				nparameters++;
				if (nparameters == 1)
					opt->address = (char*)arg.value;
				else if (nparameters == 2)
				{
					if (ft_strtol((char*)arg.value, (long*)&val) == -1)
						snprintf(errmsg, ERRMSG_MAXLEN, "Cannot handle \"packetlen\" cmdline arg \'%s\'", (char*)arg.value);
					else if (val > PACKETLEN_MAX)
						snprintf(errmsg, ERRMSG_MAXLEN, "too big packetlen %d specified", val);
					opt->packetlen = val;
				}
				else
					snprintf(errmsg, ERRMSG_MAXLEN, "Extra arg \'%s\'", (char*)arg.value);
				break ;
			case ARG_T_ERROR:
				switch (arg.errtype)
				{
					case ERR_BAD_OPTION:
						snprintf(errmsg, ERRMSG_MAXLEN, "Bad option \'-%c\'", arg.name);
						break ;
					case ERR_MISSING_PARAM:
						snprintf(errmsg, ERRMSG_MAXLEN, "Option \'-%c\' requires an argument", arg.name);
						break ;
					case ERR_BAD_PARAM_TYPE:
						snprintf(errmsg, ERRMSG_MAXLEN, "Cannot handle option \'-%c\' with arg \'%s\'", arg.name, (char*)arg.value);
						break ;
					default: // should never happen
						break;
				}
				break ;
		}
		if (*errmsg)
			return log_error(errmsg);
	}

	if (ret == -2) // code to indicate memory allocation failure
		return log_error("Out of memory");

	if (opt->first_ttl > opt->max_ttl)
		return log_error("first hop out of range");
	if (opt->squeries < 1)
		opt->squeries = 1;

	size_t min_packetlen = (opt->family == AF_INET) ? PACKETLEN_V4_MIN : PACKETLEN_V6_MIN;
	if (nparameters < 2) // packetlen was not set
		opt->packetlen = min_packetlen + 32; // default value
	else if (opt->packetlen < min_packetlen)
		opt->packetlen = min_packetlen;

	if (!opt->address)
		return (argc != 1) ? log_error("Specify \"host\" missing argument.") : print_usage(argv[0]);
	return 0;
}

int parse_waittime(char* buffer, t_waittime* result, char* errmsg)
{
	double* time = (double*)result;
	size_t start = 0;
	size_t i;

	ft_memset(result, 0, sizeof(t_waittime));
	for (i = 0; buffer[i]; i++)
	{
		if (buffer[i] == ',' || buffer[i] == '/')
		{
			char tmp = buffer[i];
			buffer[i] = '\0';

			int ret = ft_strtof(buffer + start, time);
			buffer[i] = tmp;
			if (ret == -1)
				return sprintf(errmsg, "Cannot handle \'-w\' option with arg \'%s\'", buffer);
			
			start = i + 1;
			time++;
		}
	}
	if (ft_strtof(buffer + start, time) != 0)
		return sprintf(errmsg, "Cannot handle \'-w\' option with arg \'%s\'", buffer);

	if (result->max < 0. || result->here < 0. || result->near < 0.)
		return sprintf(errmsg, "bad wait specifications \'%.3f,%.3f,%.3f\' used", result->max, result->here, result->near);

	return 0;
}

int print_usage(const char* program_name)
{
	dprintf(STDERR_FILENO,
		"Usage:\n"
		" %s [ -6In ][ -f first_ttl ] [ -m max_ttl ] [ -N squeries ] [ -p port ]"
		"[ -w MAX,HERE,NEAR ] [ -q nqueries ] host [ packetlen ]\n\n"
		"Options:\n"
		" -6 \t\t\tUse IPv6\n"
		" -f first_ttl\t\tStart from the first_ttl hop (instead of 1)\n"
		" -I \t\t\tUse ICMP ECHO for tracerouting\n"
		" -m max_ttl\t\tSet the max number of hops (max TTL to be reached). Default is 30\n"
		" -N squeries\t\tSet the number of probes to be tried simultaneously (default is 16)\n"
		" -n \t\t\tDo not resolve IP addresses to their domain names\n"
		" -p port\t\tSet the destination port to use. It is either initial udp port value for \"default\"\n"
		"\t\t\tmethod(incremented by each probe, default is 33434),\n"
		"\t\t\tor initial seq for \"icmp\" (incremented as well, default from 1)\n"
		" -w max[,here,near]\tDetermines how long to wait for a response to a probe.\n"
		"\t\t\tmax specifies the maximum time (in seconds, default 5.0) to wait, in any case.\n"
		"\t\t\there (default 3.0) specifies a factor to multiply the round trip time of an already received response from the same hop.\n"
		"\t\t\tthe resulting value is used as a timeout for the probe, instead of (but no more than) max.\n"
		"\t\t\tnear (default 10.0) specifies a similar factor for a response from some next hop.\n"
		" -q nqueries\tSet the number of probes per each hop. Default is 3\n",
		program_name
	);
	exit(EXIT_SUCCESS);
}
