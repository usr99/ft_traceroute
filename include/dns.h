/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dns.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/20 20:19:12 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/23 22:11:30 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _DNS_H_
# define _DNS_H_

# include "options.h"

int resolve_hostname(struct sockaddr_storage* host, t_cmdline_args* opt);

#endif
