/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/18 10:13:43 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/20 20:23:20 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>

#include "ft_traceroute.h"
#include "dns.h"

int main(int argc, char** argv)
{
	t_cmdline_args opt;
	if (parse_arguments(argc, argv, &opt) == -1)
		return 2;

	struct addrinfo hostname;
	if (resolve_hostname(&hostname, &opt) == -1)
		return 2;

	int sockfd;
	if ((sockfd = socket(hostname.ai_family, hostname.ai_socktype, hostname.ai_protocol)) == -1)
	{
		log_error("failed to create socket");
		return 2;
	}

	// char buf[] = "                     ";
	// if (host->ai_family == AF_INET)
	// 	((struct sockaddr_in*)host->ai_addr)->sin_port = htons(opt.port);
	// else
	// 	((struct sockaddr_in6*)host->ai_addr)->sin6_port = htons(opt.port);
	// if (sendto(sockfd, buf, sizeof(buf), 0, host->ai_addr, host->ai_addrlen) == -1)
	// 	return log_error("failed to send datagram");

	// free(host);
	close(sockfd);
	return 0;
}
