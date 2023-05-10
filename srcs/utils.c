/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 16:43:23 by chulee            #+#    #+#             */
/*   Updated: 2023/05/10 10:35:37 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "flow_simulator.h"

long	get_file_size(FILE *file)
{
	long current_position, file_size;

	current_position = ftell(file);
	fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, current_position, SEEK_SET);
    return (file_size);
}

struct session_simulator*	get_simulator(void)
{
	static struct session_simulator	*ret;
	int								i;

	if (ret == NULL)
	{
		ret = malloc(sizeof(struct session_simulator));
		assert(ret != NULL);
		memset(ret, 0, sizeof(struct session_simulator));
		for (i = 0; i < BUFF_LENGTH; i++)
		{
			ret->buffers[i].status = EMPTY;
			pthread_mutex_init(&ret->buffers[i].lock, NULL);
		}
		pthread_mutex_init(&ret->lock, NULL);
	}
	return (ret);
}
