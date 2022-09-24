/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   probes.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/21 13:43:57 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/23 19:19:40 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

#include "ft_traceroute.h"
#include "probes.h"

void init_probe(t_probe* ptr, uint16_t id)
{
	ptr->id = id;
	gettimeofday(&ptr->time_sent, NULL);
}

void extract_probe_info(const char* packetbuf, uint8_t* ttl, uint16_t* id)
{
	const struct iphdr* ip = (const struct iphdr*)packetbuf;
	const struct udphdr* udp = (const struct udphdr*)(ip + 1);

	*ttl = ip->ttl;
	*id = ntohs(udp->dest);
}

int save_gateway(t_hop* hop, const struct sockaddr* addr, uint16_t id)
{
	t_probe* p = hop->probes + (id - hop->probes->id);

	gettimeofday(&p->time_recvd, NULL);
	if (inet_ntop(AF_INET, &((struct sockaddr_in*)addr)->sin_addr, p->address, INET_ADDRSTRLEN) == NULL)
		return -1;
	if (getnameinfo(addr, sizeof(struct sockaddr_in), p->hostname, HOST_NAME_MAX, NULL, 0, 0) != 0)
		return -1;
	return 0;	
}
