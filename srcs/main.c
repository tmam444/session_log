/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:56:45 by chulee            #+#    #+#             */
/*   Updated: 2023/04/14 14:37:11 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"

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
		filename = make_filename(&m_data->time_info, i);
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
	unsigned long long	start_time, end_time, int_per_second_byte, ext_per_second_byte, diff_time;

	start_time = data->start_time < 60 ? data->start_time : 0;
	end_time = data->end_time < 60 ? data->end_time : 59;
	int_per_second_byte = data->int_byte.byte;
	ext_per_second_byte = data->ext_byte.byte;
	diff_time = end_time - start_time;
	if (diff_time > 0)
	{
		if (int_per_second_byte > 0)
			int_per_second_byte = int_per_second_byte / diff_time;
		if (ext_per_second_byte > 0)
			ext_per_second_byte = ext_per_second_byte / diff_time;
	}
	while (start_time <= end_time)
	{
		m_data->s_data[start_time].internal[data->int_cid].total_byte += int_per_second_byte;
		assert(m_data->s_data[start_time].internal[data->int_cid].total_byte >= int_per_second_byte);
		m_data->s_data[start_time].external[data->ext_cid].total_byte += ext_per_second_byte;
		assert(m_data->s_data[start_time].external[data->ext_cid].total_byte >= ext_per_second_byte);
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
	int					i, buffer_id, read_size;
	bool				new_file;
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
		new_file = true;
		while (1)
		{
			buffer_id = (buffer_id + 1) % BUFF_LENGTH;
			pthread_mutex_lock(&file_data->buffers[buffer_id].lock);
			read_size = fread(file_data->buffers[buffer_id].b_data, 1, BUFF_SIZE, fp);
			if (new_file)
			{
				file_data->buffers[buffer_id].status = NEW;
				new_file = false;
			}
			else if (read_size == 0)
				file_data->buffers[buffer_id].status = END;
			else
				file_data->buffers[buffer_id].status = CONTINUE;
			file_data->buffers[buffer_id].read_size = read_size;
			pthread_mutex_unlock(&file_data->buffers[buffer_id].lock);
			if (read_size == 0)
				break;
		}
		fclose(fp);
	}
	pthread_mutex_lock(&file_data->lock);
	file_data->read_end = true;
	pthread_mutex_unlock(&file_data->lock);
	return (EXIT_SUCCESS);
}

void	read_header(struct buffer *buff, struct tm *time_info, int *buffer_index)
{
	struct RawFileHeader2_t	header;
	time_t					temp_time;
	struct tm				*t;
	
	assert(time_info != NULL && buff != NULL);
	memcpy(&header, buff->b_data, HEADER_SIZE);
	temp_time = header.time;
	t = localtime(&temp_time);
	memcpy(time_info, t, sizeof(struct tm));
	*buffer_index += HEADER_SIZE;
}

void	add_data(struct minute_data *m_data, void *data)
{
	struct RawDataVer2_t	*r_data = data;

	save_data(r_data, m_data);
}

void	read_data(struct buffer *buff, int *buffer_index, struct minute_data *m_data)
{
	static int					remaining_buffer_size;
	static unsigned char		data[DATA_SIZE];
	int							i, read_size;

	if (remaining_buffer_size != 0)
	{
		memcpy(data + DATA_SIZE - remaining_buffer_size, buff->b_data, remaining_buffer_size);
		add_data(m_data, data);
		*buffer_index += remaining_buffer_size;
	}
	for (i = *buffer_index; i < buff->read_size; i += DATA_SIZE)
	{
		if (BUFF_SIZE - i < (int)DATA_SIZE)
			read_size = BUFF_SIZE - i;
		else
			read_size = DATA_SIZE;
		memcpy(data, buff->b_data + i, read_size);
		if (read_size == DATA_SIZE)
			add_data(m_data, data);
	}
	remaining_buffer_size = i - buff->read_size;
}

void*	write_thread(void *arg)
{
	struct save_file		*file_data = arg;
	bool					is_end = false;
	int						minute_index, buffer_id, buffer_index;
	int						i;

	minute_index = buffer_id = 0;
	while (!is_end)
	{
		buffer_index = 0;
		buffer_id = (buffer_id + 1) % BUFF_LENGTH;
		pthread_mutex_lock(&file_data->buffers[buffer_id].lock);
		if (file_data->buffers[buffer_id].read_size != 0)
		{
			if (file_data->buffers[buffer_id].status == NEW)
				read_header(&file_data->buffers[buffer_id], &file_data->m_data[minute_index].time_info, &buffer_index);
			read_data(&file_data->buffers[buffer_id], &buffer_index, &file_data->m_data[minute_index]);
		}
		if (file_data->buffers[buffer_id].status == END || file_data->buffers[buffer_id].read_size == 0)
		{
			if (file_data->buffers[buffer_id].status == END)
				minute_index++;
			pthread_mutex_lock(&file_data->lock);
			is_end = file_data->read_end;
			pthread_mutex_unlock(&file_data->lock);
		}
		file_data->buffers[buffer_id].status = CONTINUE;
		file_data->buffers[buffer_id].read_size = 0;
		pthread_mutex_unlock(&file_data->buffers[buffer_id].lock);
	}
	// file create
	for (i = 0; i < minute_index; i++)
		save_second_file(&file_data->m_data[i]);
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
	pthread_create(&write_thread_id, NULL, write_thread, file_data);
	pthread_join(read_thread_id, NULL);
	pthread_join(write_thread_id, NULL);
	clear(file_data);
	return (EXIT_SUCCESS);
}
