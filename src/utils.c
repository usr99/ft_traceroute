/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/19 21:51:30 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/29 16:30:30 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "ft_traceroute.h"

int log_error(const char* message)
{
	dprintf(STDERR_FILENO, "%s\n", message);
	return -1;
}

float get_duration_from_now(struct timeval* from)
{
	struct timeval now;

	gettimeofday(&now, NULL);
	return get_duration_ms(from, &now);
}

float get_duration_ms(struct timeval* from, struct timeval* to)
{
	float seconds;
	float milliseconds;
	
	seconds = (to->tv_sec - from->tv_sec) * 1000;
	milliseconds = (to->tv_usec - from->tv_usec) / 1000.f;
	return seconds + milliseconds;
}

int addr_to_text(const struct sockaddr_storage* addr, char* buffer)
{
	const char* ret;
	if (addr->ss_family == AF_INET)
		ret = inet_ntop(AF_INET, &((struct sockaddr_in*)addr)->sin_addr.s_addr, buffer, INET6_ADDRSTRLEN);
	else
		ret = inet_ntop(AF_INET6, ((struct sockaddr_in6*)addr)->sin6_addr.__in6_u.__u6_addr32, buffer, INET6_ADDRSTRLEN);
	return ret ? 0 : -1;
}

__int128_t get_address(struct sockaddr_storage* address)
{
	if (address->ss_family == AF_INET)
		return ((struct sockaddr_in*)address)->sin_addr.s_addr;
	else
	{
		uint32_t* addr6 = ((struct sockaddr_in6*)address)->sin6_addr.__in6_u.__u6_addr32;
		__int128_t result = addr6[0] + addr6[1] + addr6[2] + addr6[3];
		return result;
	}
}
