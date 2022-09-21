/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   probes.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mamartin <mamartin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/21 13:43:57 by mamartin          #+#    #+#             */
/*   Updated: 2022/09/21 19:34:40 by mamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <netdb.h>
#include <sys/time.h>

#include "ft_traceroute.h"
#include "probes.h"

void* push_back(t_list** root, size_t size)
{
	void* data = ft_calloc(1, size);
	if (!data)
		return NULL;

	t_list* new_node = ft_lstnew(data);
	if (!new_node)
	{
		free(data);
		return NULL;
	}

	ft_lstadd_back(root, new_node);
	return data;
}

t_ttl_settings* push_new_ttl_setting(t_list** queries, uint8_t ttl)
{
	t_ttl_settings* data = push_back(queries, sizeof(t_ttl_settings));
	if (data)
		data->ttl = ttl;
	return data;
}

t_probe_packet* push_new_probe_packet(t_list** probes, uint16_t id)
{
	t_probe_packet* data = push_back(probes, sizeof(t_probe_packet));
	if (data)
	{
		data->id = id;
		gettimeofday(&data->time_sent, NULL);
	}
	return data;
}

t_gateway_response* push_new_response(t_list** gateways, struct sockaddr* addr, struct timeval timestamp)
{
	t_gateway_response* data = push_back(gateways, sizeof(t_gateway_response));
	if (data)
	{
		if (inet_ntop(AF_INET, &((struct sockaddr_in*)addr)->sin_addr, data->address, INET_ADDRSTRLEN) == NULL)
			return NULL;
		if (getnameinfo(addr, sizeof(struct sockaddr_in), data->name, HOST_NAME_MAX, NULL, 0, 0) != 0)
			return NULL;
		data->time = timestamp;
	}
	return data;
}

t_probe_packet* get_probe_from_id(t_list* queries, uint16_t id)
{
	t_list* query;

	for (query = queries; query; query = query->next)
	{
		t_list* probe;
		for (probe = ((t_ttl_settings*)query->content)->probes; probe; probe = probe->next)
		{
			t_probe_packet* probe_info = probe->content;
			if (probe_info->id == id) // UDP destination port is used to match responses to probes
				return probe_info;
		}
	}
	return NULL;
}

void debug_queries(t_list* queries)
{
	while (queries)
	{
		t_ttl_settings* query = queries->content;
		printf("TTL %d\n", query->ttl);
		printf("%d sent / %d received\n", query->nb_sent, query->nb_recvd);

		while (query->probes)
		{
			t_probe_packet* probe = query->probes->content;
			printf("%d\n", probe->id);

			while (probe->replies)
			{
				t_gateway_response* reply = probe->replies->content;
				printf("%s (%s) %.3fms\n", reply->name, reply->address, get_duration_ms(&probe->time_sent, &reply->time));

				probe->replies = probe->replies->next;
			}
			query->probes = query->probes->next;
		}
		queries = queries->next;
		printf("\n");
	}
}
