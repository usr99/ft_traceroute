/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/18 10:13:43 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/23 16:13:13 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>

#include "ft_traceroute.h"
#include "probes.h"
#include "dns.h"

int main(int argc, char** argv)
{
	t_cmdline_args opt;
	if (parse_arguments(argc, argv, &opt) == -1)
		return 2;

	debug_options(&opt);
	return 0;

	// struct addrinfo host;
	// if (resolve_hostname(&host, &opt) == -1)
	// 	return 2;

	// int sockfd;
	// if ((sockfd = create_socket(&host)) == -1)
	// 	return 2;

	// int icmp_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	// if (icmp_sock == -1)
	// {
	// 	perror("socket");
	// 	return 2;
	// }

	// int ret;
	// t_list* queries = NULL;
	// while (opt.ttl <= opt.max_ttl)
	// {
	// 	if (send_probes(sockfd, &host, &opt, &queries) != 0)
	// 	{
	// 		// behavior not defined yet
	// 		return 2;
	// 	}
		
	// 	if ((ret = recv_response(icmp_sock, &opt, &queries)) == -1)
	// 	{
	// 		// behavior not defined yet
	// 		return 2;
	// 	}
	// 	else if (ret == 1)
	// 	{
	// 		// some responses got timed out
	// 		printf("Somes probes got timed out\n");
	// 	}
	// 	else
	// 	{
	// 		debug_queries(queries);
	// 		// received all responses
	// 	}
	// }

	// close(sockfd);
	// close(icmp_sock);
	// // ft_lstclear(queries);
	return 0;
}

// int create_socket(const struct addrinfo* host)
// {
// 	int sockfd;
// 	int ret;

// 	if ((sockfd = socket(host->ai_family, host->ai_socktype, host->ai_protocol)) != -1)
// 	{
// 		if (host->ai_protocol == IPPROTO_UDP)
// 		{
// 			if (host->ai_family == AF_INET)
// 			{
// 				struct sockaddr_in addr = {
// 					.sin_family = AF_INET,
// 					.sin_addr.s_addr = INADDR_ANY,
// 					.sin_port = 0,
// 				};
// 				ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
// 			}
// 			else
// 			{
// 				struct sockaddr_in6 addr = {
// 					.sin6_family = AF_INET6,
// 					.sin6_addr = in6addr_any,
// 					.sin6_port = 0,
// 				};
// 				ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in6));
// 			}

// 			if (ret == -1)
// 			{
// 				log_error("failed to bind socket");
// 				close(sockfd);
// 				sockfd = -1;
// 			}
// 		}
// 	}
// 	else
// 		log_error("failed to create socket");
// 	return sockfd;
// }

// int send_probes(int sockfd, const struct addrinfo* host, t_cmdline_args* opt, t_list** queries)
// {
// 	t_list* node = *queries;
// 	while (node && ((t_ttl_settings*)node->content)->ttl != opt->ttl)
// 		node = node->next;
// 	t_ttl_settings* query = (node ? node->content : NULL);

// 	int sent = 0;
// 	while (sent < opt->squeries)
// 	{
// 		if (query == NULL) // no query exist with this ttl
// 		{
// 			/* Push a query with the current ttl value */
// 			if (!(query = push_new_ttl_setting(queries, opt->ttl)))
// 				return -1;
// 			/* Set ttl field for the next packets sent */
// 			if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &opt->ttl, sizeof(uint8_t)) == -1)
// 				return -1;
// 		}

// 		/* Send "nqueries" packets to the destination host */
// 		while (query->nb_sent < opt->nqueries && sent < opt->squeries)
// 		{
// 			/*
// 			** Set destination port
// 			** we don't want the host to process the probe
// 			** so we send it on an "unlikely" port
// 			** increased for each probe, can be used to match probes to responses
// 			*/
// 			if (host->ai_family == AF_INET)
// 				((struct sockaddr_in*)host->ai_addr)->sin_port = htons(opt->port);
// 			else
// 				((struct sockaddr_in6*)host->ai_addr)->sin6_port = htons(opt->port);

// 			if (sendto(sockfd, NULL, 0, 0, host->ai_addr, host->ai_addrlen) >= 0)
// 			{
// 				if (!push_new_probe_packet(&query->probes, opt->port))
// 					return -1;
// 				opt->port++;
// 			}
// 			else
// 				return -1;

// 			sent++;
// 			query->nb_sent++;
// 		}

// 		/* Increase ttl after all packets were sent */
// 		if (query->nb_sent == opt->nqueries)
// 			opt->ttl++;

// 		query = NULL; // needed to create a new query on next iteration
// 	}
// 	return 0;
// }

// int recv_response(int icmp_sock, const t_cmdline_args* opt, t_list** queries)
// {
// 	fd_set rfds;
// 	struct timeval timestamp;
// 	struct timeval timeout = {
// 		.tv_sec = 5, .tv_usec = 0
// 	};
// 	int count = 0;

// 	while (count < opt->squeries)
// 	{
// 		/*
// 		** Block until there is input to read from ICMP socket
// 		** or until the timeout expires (defined by -w option)
// 		*/
// 		int ret;
// 		FD_ZERO(&rfds);
// 		FD_SET(icmp_sock, &rfds);
// 		if ((ret = select(icmp_sock + 1, &rfds, NULL, NULL, &timeout)) == -1)
// 			return -1;
// 		if (ret == 0)
// 			return 1; // no replies after timeout
// 		gettimeofday(&timestamp, NULL);

// 		/* Receive ICMP message */
// 		char buf[100];
// 		struct sockaddr_storage address;
// 		socklen_t addrlen = sizeof(address);
// 		ssize_t bytes = recvfrom(icmp_sock, buf, sizeof(buf), 0, (struct sockaddr*)&address, &addrlen);
// 		if (bytes == -1)
// 			return -1;

// 		/*
// 		** Match one of the probes sent to the response
// 		** UDP destination port is used to identify probes
// 		**
// 		** original packet can be found after IP and ICMP headers
// 		** UDP header is placed after the IP header in the original packet
// 		*/
// 		struct udphdr* udp = (struct udphdr*)(buf + sizeof(struct iphdr) * 2 + sizeof(struct icmphdr));
// 		t_probe_packet* probe = get_probe_from_id(*queries, ntohs(udp->dest));
// 		if (!probe)
// 			return -1;

// 		// update query->nb_recvd

// 		push_new_response(&probe->replies, (struct sockaddr*)&address, timestamp);
// 		count++;
// 	}

// 	return 0;	
// }
