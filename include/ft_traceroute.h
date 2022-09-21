/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/18 10:17:35 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/21 19:16:58 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _FT_TRACEROUTE_H_
# define _FT_TRACEROUTE_H_

# include "options.h"
# include "libft.h"

int create_socket(const struct addrinfo* host);
int send_probes(int sockfd, const struct addrinfo* host, t_cmdline_args* opt, t_list** queries);
int recv_response(int icmp_sock, const t_cmdline_args* opt, t_list** queries);

int log_error(const char* message);
float get_duration_from_now(struct timeval* from);
float get_duration_ms(struct timeval* from, struct timeval* to);

#endif
