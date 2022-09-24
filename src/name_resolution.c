/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   name_resolution.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/20 20:19:46 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/24 14:35:10 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"
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

int fetch_hostname(const struct sockaddr* addr, t_probe* gateway)
{
	if (inet_ntop(AF_INET, &((struct sockaddr_in*)addr)->sin_addr, gateway->address, INET_ADDRSTRLEN) == NULL)
		return -1;
	if (getnameinfo(addr, sizeof(struct sockaddr_in), gateway->hostname, HOST_NAME_MAX, NULL, 0, 0) != 0)
		return -1;
	return 0;
}
