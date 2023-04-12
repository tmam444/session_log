/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:56:45 by chulee            #+#    #+#             */
/*   Updated: 2023/04/12 19:09:34 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"
#include <pthread.h>
#include <stdlib.h>

static char*	make_filename(struct tm *time_info, int second)
{
	const char	*path = "./log_file/";
	const int	filename_len = 100;
	char		*filename;

	filename = malloc(filename_len);
	assert(filename != NULL);
	memset(filename, 0, filename_len);
	snprintf(filename, filename_len, "%straffic_%04d%02d%02d_%02d%02d%02d.txt",
			path, time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday,
			time_info->tm_hour, time_info->tm_min, second);
	return (filename);
}

void	save_second_file(struct minute_data *m_data)
{
	char		*filename;
	FILE		*fp;
	int			i, j;

	for (i = 0; i < SECOND; i++)
	{
		filename = make_filename(m_data->time_info, i);
		fp = fopen(filename, "w");
		for (j = 0; j < MAX_CID_SIZE; j++)
			if (m_data->s_data[i].internal[j].total_byte != 0)
				fprintf(fp, "%d,%llu,%llu\n", j, m_data->s_data[i].internal[j].total_byte, m_data->s_data[i].external[j].total_byte);
		free(filename);
		fclose(fp);
	}
}

void	save_data(struct RawDataVer2_t *data, struct minute_data *m_data)
{
	int	start_time, end_time, int_per_second_byte, ext_per_second_byte;

	start_time = data->start_time < 60 ? data->start_time : 0;
	end_time = data->end_time < 60 ? data->end_time : 59;
	int_per_second_byte = data->int_byte.byte;
	ext_per_second_byte = data->ext_byte.byte;
	if (end_time - start_time != 0)
	{
		int_per_second_byte /= (end_time - start_time);
		ext_per_second_byte /= (end_time - start_time);
	}
	while (start_time <= end_time)
	{
		m_data->s_data[start_time].internal[data->int_cid].total_byte += int_per_second_byte;
		assert(m_data->s_data[start_time].internal[data->int_cid].total_byte >= (unsigned long long)int_per_second_byte);
		m_data->s_data[start_time].external[data->ext_cid].total_byte += ext_per_second_byte;
		assert(m_data->s_data[start_time].external[data->ext_cid].total_byte >= (unsigned long long)ext_per_second_byte);
		start_time++;
	}
}

struct save_file*	setup(int argc, char *argv[])
{
	struct save_file		*ret;
	int						i;

	ret = malloc(sizeof(struct save_file));
	assert(ret != NULL);
	memset(ret, 0, sizeof(struct save_file));
	ret->m_data = malloc(sizeof(struct minute_data) * argc);
	assert(ret->m_data != NULL);
	memset(ret->m_data, 0, sizeof(struct minute_data) * argc);
	for (i = 0; i < BUFF_LENGTH; i++)
		pthread_mutex_init(&ret->buffers[i].lock, NULL);
	pthread_mutex_init(&ret->lock, NULL);
	ret->log_files = argv;
	ret->file_count = argc;
	return (ret);
}

void*	read_thread(void *arg)
{
	struct save_file	*file_data = arg;
	int					i, j, buffer_id, read_size, print_size;
	FILE				*fp;

	buffer_id = 0;
	for (i = 1; i < file_data->file_count; i++)
	{
		fp = fopen(file_data->log_files[i], "r");
		if (fp == NULL)
		{
			fprintf(stderr, "Error!\nFile Open Error : %s\n", file_data->log_files[i]);
			exit(EXIT_FAILURE);
		}
		while (1)
		{
			pthread_mutex_lock(&file_data->buffers[buffer_id].lock);
			read_size = fread(file_data->buffers[buffer_id].b_data, BUFF_SIZE, 1, fp);
			pthread_mutex_unlock(&file_data->buffers[buffer_id].lock);
			if (read_size == 1)
				print_size = BUFF_SIZE;
			else
				print_size = read_size;
			for (j = 0; j < print_size; j++)
			{
				printf("%02X ", file_data->buffers->b_data[j]);
				if (j % 16 == 0)
					printf("\n");
			}
			buffer_id = (buffer_id + 1) % BUFF_LENGTH;
			if (read_size != 0)
			{
				file_data->buffers[buffer_id].status = END;
				break ;
			}
		}
	}
	file_data->read_end = true;
	return (EXIT_SUCCESS);
}

void*	write_thread(void *arg)
{
	struct save_file	*file_data = arg;
	int					i, buffer_id, read_size, index;

	buffer_id = 0;
	while (1)
	{
		pthread_mutex_lock(&file_data->lock);
		if (file_data->read_end)
			break ;
		pthread_mutex_unlock(&file_data->lock);
		pthread_mutex_lock(&file_data->buffers[buffer_id].lock);
		pthread_mutex_unlock(&file_data->buffers[buffer_id].lock);
	}
	return (EXIT_SUCCESS);
}

void	clear(struct save_file *file_data)
{
	int	i;

	if (file_data)
	{
		if (file_data->m_data)
			free(file_data->m_data);
		for (i = 0; i < BUFF_LENGTH; i++)
			pthread_mutex_destroy(&file_data->buffers[i].lock);
		free(file_data);
	}
}

int	main(int argc, char *argv[])
{
	pthread_t			read_thread_id, write_thread_id;
	struct save_file	*file_data;

	if (argc == 1)
	{
		fprintf(stderr, "Error!\nUsage : %s filename1 filename2 ...\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	file_data = setup(argc, argv);
	pthread_create(&read_thread_id, NULL, read_thread, file_data);
	// pthread_create(&write_thread_id, NULL, write_thread, file_data);
	pthread_join(read_thread_id, NULL);
	// pthread_join(write_thread_id, NULL);
	clear(file_data);
	return (EXIT_SUCCESS);
}
