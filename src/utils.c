/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/19 21:51:30 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/21 19:18:31 by mamartin         ###   ########.fr       */
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
