/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dns.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/20 20:19:46 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/23 22:14:48 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"
#include "dns.h"
#include "libft.h"

int resolve_hostname(struct sockaddr_storage* host, t_cmdline_args* opt)
{
	struct addrinfo *results;
	struct addrinfo hint = {
		.ai_family = opt->family,
		.ai_socktype = opt->socktype,
		.ai_protocol = opt->protocol,
		.ai_flags = AI_ADDRCONFIG
	};

	int code;
	if ((code = getaddrinfo(opt->address, NULL, &hint, &results)) != 0)
		return log_error(gai_strerror(code));
		
	ft_memcpy(host, results->ai_addr, results->ai_addrlen);
	freeaddrinfo(results);
	return 0;
}
