/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_thread.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 14:33:05 by chulee            #+#    #+#             */
/*   Updated: 2023/04/24 16:43:40 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"

void*	read_thread(void *arg)
{
	struct session_simulator	*s_simulator = arg;
	int							buffer_id, read_size, file_size;
	List						*cur;
	FILE						*fp;
	error_code					*err_code;

	err_code = malloc(sizeof(error_code));
	assert(err_code != NULL);
	*err_code = NONE;
	buffer_id = 0;
	for (cur = s_simulator->log_files; cur != NULL; cur = cur->next)
	{
		fp = fopen(cur->value, "r");
		if (fp == NULL)
		{
			log_message(LOG_ERROR, "File Open : %s\n", (char *)cur->value);
			*err_code = ERROR_FILE_NOT_FOUND;
			break;
		}
		file_size = get_file_size(fp);
		pthread_mutex_lock(&s_simulator->buffers[buffer_id].lock);
		if (s_simulator->buffers[buffer_id].status == EMPTY)
		{
			s_simulator->buffers[buffer_id].b_data = malloc(file_size);
			assert(s_simulator->buffers[buffer_id].b_data != NULL);
			read_size = fread(s_simulator->buffers[buffer_id].b_data, file_size, 1, fp);
			DEBUG_LOG("Read File - %s, Size - %d, Full Read - %s", (char *)cur->value, file_size, read_size == 1 ? "true" : "false");
			if (read_size == 1)
				s_simulator->buffers[buffer_id].read_size = file_size;
			else
				s_simulator->buffers[buffer_id].read_size = read_size;
			s_simulator->buffers[buffer_id].status = NEW;
		}
		pthread_mutex_unlock(&s_simulator->buffers[buffer_id].lock);
		fclose(fp);
		buffer_id = (buffer_id + 1) % BUFF_LENGTH;
	}
	pthread_mutex_lock(&s_simulator->buffers[buffer_id].lock);
	s_simulator->buffers[buffer_id].status = END;
	pthread_mutex_unlock(&s_simulator->buffers[buffer_id].lock);
	DEBUG_LOG("read_thread end");
	return (err_code);
}
