/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_thread.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 14:33:05 by chulee            #+#    #+#             */
/*   Updated: 2023/04/18 17:34:46 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"

long	get_file_size(FILE *file) {
	long current_position, file_size;

	current_position = ftell(file);
	fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, current_position, SEEK_SET);
    return (file_size);
}

void*	read_thread(void *arg)
{
	struct save_file	*file_data = arg;
	int					buffer_id, read_size, file_size;
	List				*cur;
	FILE				*fp;

	buffer_id = 0;
	cur = file_data->log_files;
	while (cur != NULL)
	{
		fp = fopen(cur->value, "r");
		if (fp == NULL)
			log_message(LOG_ERROR, "File Open : %s\n", (char *)cur->value);
		file_size = get_file_size(fp);
		buffer_id = (buffer_id + 1) % BUFF_LENGTH;
		pthread_mutex_lock(&file_data->buffers[buffer_id].lock);
		if (file_data->buffers[buffer_id].status == EMPTY)
		{
			file_data->buffers[buffer_id].b_data = malloc(file_size);
			assert(file_data->buffers[buffer_id].b_data != NULL);
			read_size = fread(file_data->buffers[buffer_id].b_data, file_size, 1, fp);
			log_message(LOG_INFO, "Read File - %s, Size - %d, Full Read - %s", (char *)cur->value, file_size, read_size == 1 ? "true" : "false");
			if (read_size == 1)
				file_data->buffers[buffer_id].read_size = file_size;
			else
				file_data->buffers[buffer_id].read_size = read_size;
			file_data->buffers[buffer_id].status = NEW;
			if (cur->next == NULL)
				file_data->buffers[buffer_id].status = END;
		}
		pthread_mutex_unlock(&file_data->buffers[buffer_id].lock);
		fclose(fp);
		cur = cur->next;
	}
	log_message(LOG_INFO, "read_thread end");
	return (EXIT_SUCCESS);
}
