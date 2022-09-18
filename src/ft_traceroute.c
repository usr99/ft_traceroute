/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/18 10:13:43 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/18 11:14:17 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "ft_traceroute.h"
#include "libft.h"

void exit_error(const char* message)
{
	dprintf(STDERR_FILENO, "ping: %s\n", message);
	exit(2);
}

t_tr_options parse_arguments(int argc, char** argv)
{
	static const t_expected_opts valid_options[N_OPTIONS_SUPPORTED] = {
		{ .name = '6', .has_param = false, .paramtype = 0 },
		{ .name = 'I', .has_param = false, .paramtype = 0 },
		{ .name = 'n', .has_param = false, .paramtype = 0 },
		{ .name = 'f', .has_param = true, .paramtype = PARAM_T_INT8 },
		{ .name = 'm', .has_param = true, .paramtype = PARAM_T_INT8 },
		{ .name = 'N', .has_param = true, .paramtype = PARAM_T_INT8 },
		{ .name = 'q', .has_param = true, .paramtype = PARAM_T_INT8 },
		{ .name = 'p', .has_param = true, .paramtype = PARAM_T_INT16 },
		{ .name = 'w', .has_param = true, .paramtype = PARAM_T_INT32 }
	};

	t_tr_options options;
	t_argument arg;
	int ret;

	while ((ret = ft_getarg(argc, argv, valid_options, N_OPTIONS_SUPPORTED, &arg)) == 0)
	{
		switch (arg.type)
		{
			case ARG_T_OPTION:
				switch (arg.info.opt.name)
				{
					case '6':
						options.bitmask &= OPT_FORCE_IPV6;
						break;
					case 'I':
						options.bitmask &= OPT_USE_ICMP;
						break;
					case 'n':
						options.bitmask &= OPT_NO_DNS_RESOLUTION;
						break;
					case 'f':
						break;
					case 'm':
						break;
					case 'N':
						break;
					case 'q':
						break;
					case 'p':
						break;
					case 'w':
						break;
					default: // should never happen
						break;
				}
				if (arg.info.opt.value)
					free(arg.info.opt.value);
				break ;
			case ARG_T_PARAMETER:
				break ;
			case ARG_T_ERROR:
				break ;
		}
	}

	if (ret == -2)
		exit_error("Out of memory");
}

int main(int argc, char** argv)
{
	return 0;
}
