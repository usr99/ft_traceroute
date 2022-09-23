/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   probes.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/21 13:19:09 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/23 16:05:28 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _PROBES_H_
# define _PROBES_H_

# include <arpa/inet.h>
# include <stdint.h>
# include <limits.h>
# include <time.h>

typedef struct s_probe
{
	/* Set after sendto() */
	uint16_t id;
	struct timeval time_sent;

	/* Set after recvfrom() */
	char hostname[HOST_NAME_MAX];
	char address[INET6_ADDRSTRLEN];
	struct timeval time_recvd;
} t_probe;

typedef struct s_hop
{
	t_probe* probes;
	unsigned int nb_sent;
	unsigned int nb_recvd;
} t_hop;

#endif
