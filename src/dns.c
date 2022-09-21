/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dns.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/20 20:19:46 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/20 20:22:43 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"
#include "dns.h"

int resolve_hostname(struct addrinfo* hostname, const t_cmdline_args* opt)
{
	int code;
	struct addrinfo *results;
	struct addrinfo hint = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = opt->socktype,
		.ai_protocol = opt->protocol,
		.ai_flags = AI_ADDRCONFIG | AI_CANONNAME
	};

	if ((code = getaddrinfo(opt->address, NULL, &hint, &results)) != 0)
		return log_error(gai_strerror(code));

	struct addrinfo* tmp;
	bool addr_found = false;
	for (tmp = results; tmp; tmp = tmp->ai_next)
	{
		if (tmp->ai_family != AF_INET && tmp->ai_family != AF_INET6)
			continue ;
		if (!addr_found || (tmp->ai_family == AF_INET6 && opt->forceIPv6))
		{
			*hostname = *tmp;
			addr_found = true;
		}
	}

	if (!addr_found)
		log_error("no hosts were found");
	freeaddrinfo(results);
	return 0;
}
