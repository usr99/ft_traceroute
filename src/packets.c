/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packets.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/21 13:43:57 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/26 17:37:25 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

#include "ft_traceroute.h"
#include "packets.h"

void init_probe(t_probe* ptr, uint16_t id)
{
	ptr->id = id;
	gettimeofday(&ptr->time_sent, NULL);
}

int process_response(char* payload, size_t len, t_hop* hops, t_config* cfg)
{
	int type = validate_packet(payload, len, cfg);
	if (type == 0)
		return 0; // ignore packet

	uint16_t id;
	struct sockaddr_in addr;
	parse_packet(payload, &addr, &id);

	size_t index = (id - hops->probes->id);
	size_t hopidx = index / cfg->opt.nqueries;
	t_hop* hop = hops + hopidx;
	t_probe* gateway = hop->probes + index % cfg->opt.nqueries;
	
	if (inet_ntop(AF_INET, &addr.sin_addr.s_addr, gateway->address, INET6_ADDRSTRLEN) == NULL)
		return -1;

	if (getnameinfo((struct sockaddr*)&addr, sizeof(struct sockaddr_in), gateway->hostname, HOST_NAME_MAX, NULL, 0, 0) != 0)
		ft_strlcpy(gateway->hostname, gateway->address, INET6_ADDRSTRLEN);
	gettimeofday(&gateway->time_recvd, NULL);

	hop->nb_recvd++;
	return (type == ICMP_PORT_UNREACH ? hopidx : 0);
}

int validate_packet(char* payload, size_t len, t_config* cfg)
{
	static size_t min_packetlen = sizeof(struct iphdr) * 2 + sizeof(struct icmphdr) + sizeof(struct udphdr);
	if (len < min_packetlen)
		return 0;

	struct iphdr* ip = (struct iphdr*)payload;
	if (compare_checksums((uint16_t*)ip, sizeof(struct iphdr), &ip->check) == -1)
		return 0;
	
	struct icmphdr* icmp = (struct icmphdr*)(ip + 1);
	if (compare_checksums((uint16_t*)icmp, len - sizeof(struct iphdr), &icmp->checksum) == -1)
		return 0;

	if (icmp->type == ICMP_DEST_UNREACH && icmp->code != ICMP_PORT_UNREACH)
		return 0;

	/* Compare original packet destination to our destination */
	struct iphdr* origin_ip = (struct iphdr*)(icmp + 1);
	if (origin_ip->daddr != ((struct sockaddr_in*)&cfg->host)->sin_addr.s_addr)
		return 0;

	return icmp->type;
}

void parse_packet(char* payload, struct sockaddr_in* addr, uint16_t* id)
{
	struct iphdr* ip = (struct iphdr*)payload;
	struct iphdr* origin_ip = (struct iphdr*)(payload + sizeof(struct iphdr) + sizeof(struct icmphdr));
	struct udphdr* origin_udp = (struct udphdr*)(origin_ip + 1);

 	/* Retrieve original destination UDP port */
	*id = ntohs(origin_udp->dest);

	/* Retrieve source address */
	ft_memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = ip->saddr;
}

uint16_t compute_checksum(uint16_t* data, size_t bytes)
{
	uint32_t sum = *data;
	size_t i;

	for (i = 1; i < bytes / 2; i++)
	{
		sum += data[i];
		if (sum >= 0x10000)
			sum -= 0xFFFF;
	}
	return ~sum;
}

int compare_checksums(uint16_t* data, size_t bytes, uint16_t* cs)
{
	uint16_t oldcs = *cs;
	*cs = 0;

	uint16_t newcs = compute_checksum(data, bytes);
	return (newcs == oldcs);
}
