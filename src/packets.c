/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packets.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/21 13:43:57 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/29 23:34:50 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

#include "ft_traceroute.h"
#include "packets.h"

#include <netinet/ip6.h>

void init_probe(t_probe* ptr, uint16_t id)
{
	ptr->status = WAITING_REPLY;
	ptr->id = id;
	gettimeofday(&ptr->time_sent, NULL);
}

int validate_packet(char* payload, size_t len, t_config* cfg)
{
	/*
	** Packets should have at least IP + ICMP headers
	** with IP + UDP headers from original datagram
	*/
	static const size_t min_packetlen_ipv4 = sizeof(struct iphdr) * 2 + sizeof(struct icmphdr) + sizeof(struct udphdr);
	static const size_t min_packetlen_ipv6 = sizeof(struct ip6_hdr) + sizeof(struct udphdr);

	size_t sizehdr;
	if (cfg->host.ss_family == AF_INET)
	{
		if (len < min_packetlen_ipv4)
			return 0;
		sizehdr = sizeof(struct iphdr);

		/* Check IPv4 header checksum */
		struct iphdr* ip = (struct iphdr*)payload;
		if (compare_checksums((uint16_t*)ip, sizeof(struct iphdr), &ip->check) == -1)
			return 0;		

		/* Compare original packet destination to our destination */
		struct iphdr* origin_ip = (struct iphdr*)(payload + sizehdr + sizeof(struct icmphdr));
		if (origin_ip->daddr != get_address(&cfg->host))
			return 0;
	}
	else
	{
		if (len < min_packetlen_ipv6)
			return 0;
		sizehdr = sizeof(struct ip6_hdr);

		/* Compare original packet destination to our destination */
		struct ip6_hdr* origin_ip = (struct ip6_hdr*)(payload + sizehdr + sizeof(struct icmp6_hdr));
		if (*(__int128_t*)origin_ip->ip6_dst.__in6_u.__u6_addr32 != get_address(&cfg->host))
			return 0;
	}

	/*
	** Process ICMP header
	** discard all messages that are not TIME_EXCEEDED or PORT_UNREACHABLE
	*/
	struct icmphdr* icmp = (struct icmphdr*)(payload + sizehdr);
	if (compare_checksums((uint16_t*)icmp, len - sizehdr, &icmp->checksum) == -1)
		return 0;
	if (icmp->type == ICMP_DEST_UNREACH && icmp->code != ICMP_PORT_UNREACH)
		return 0;
	return icmp->type;
}

void parse_packet(char* payload, struct sockaddr_storage* addr, uint16_t* id)
{
	struct udphdr* origin_udp;

	if (addr->ss_family == AF_INET)
	{
		struct iphdr* ip = (struct iphdr*)payload;
		struct iphdr* origin_ip = (struct iphdr*)(payload + sizeof(struct iphdr) + sizeof(struct icmphdr));
		origin_udp = (struct udphdr*)(origin_ip + 1);
		((struct sockaddr_in*)addr)->sin_addr.s_addr = ip->saddr; // retrieve source address
	}
	else
	{
		struct icmp6_hdr* icmp6 = (struct icmp6_hdr*)payload;
		struct ip6_hdr* origin_ip = (struct ip6_hdr*)(icmp6 + 1);
		origin_udp = (struct udphdr*)(origin_ip + 1);
		*(__int128_t*)((struct sockaddr_in6*)addr)->sin6_addr.__in6_u.__u6_addr32 = *(__int128_t*)ip->ip6_src.__in6_u.__u6_addr32; // retrieve source address
	}

 	/* Retrieve original destination UDP port */
	*id = ntohs(origin_udp->dest);
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
