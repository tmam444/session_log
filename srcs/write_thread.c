/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   write_thread.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 14:33:19 by chulee            #+#    #+#             */
/*   Updated: 2023/04/18 15:47:29 by chulee           ###   ########.fr       */
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

static void	save_second_file(struct minute_data *m_data)
{
	struct tm	empty_tm = {0};
	char		*filename;
	FILE		*fp;
	int			i, j;

	if (memcmp(&empty_tm, &m_data->time_info, sizeof(struct tm)) == 0)
		return;
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

static void	save_data(void *buffer, struct minute_data *m_data)
{
	struct RawDataVer2_t	*data = buffer;
	unsigned long long		start_time, end_time, int_per_second_byte, ext_per_second_byte, diff_time;

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

static int	read_header(unsigned char *buff, struct save_file *file_info, int *buffer_index)
{
	struct RawFileHeader2_t	*header = (struct RawFileHeader2_t *)buff;
	int						minute_index;
	time_t					temp_time, start_time;
	
	assert(file_info != NULL && buff != NULL && buffer_index != NULL);
	start_time = mktime(&file_info->start_tm);
	temp_time = header->time;
	minute_index = ((temp_time - start_time) / 60);
	if (minute_index < 0 || minute_index >= 2)
		log_message(LOG_ERROR, "File Header Wrong!");
	localtime_r(&temp_time, &file_info->m_data[minute_index].time_info);
	*buffer_index += HEADER_SIZE;
	return (minute_index);
}

static void	read_data(struct buffer *buff, int *buffer_index, struct minute_data *m_data)
{
	static int					remaining_buffer_size;
	static unsigned char		remaining_data[DATA_SIZE];
	int							i, read_size;

	if (remaining_buffer_size != 0)
	{
		memcpy(remaining_data + DATA_SIZE - remaining_buffer_size, buff->b_data, remaining_buffer_size);
		save_data(remaining_data, m_data);
		*buffer_index += remaining_buffer_size;
	}
	for (i = *buffer_index; i < buff->read_size; i += DATA_SIZE)
	{
		if (buff->read_size - i < (int)DATA_SIZE)
		{
			read_size = buff->read_size - i;
			memcpy(remaining_data, buff->b_data + i, read_size);
		}
		else
			read_size = DATA_SIZE;
		if (read_size == DATA_SIZE)
			save_data(buff->b_data + i, m_data);
	}
	remaining_buffer_size = i - buff->read_size;
}

void*	write_thread(void *arg)
{
	struct save_file		*file_data = arg;
	int						minute_index, buffer_id, buffer_index;
	int						i;

	buffer_id = 0;
	while (true)
	{
		buffer_index = 0;
		buffer_id = (buffer_id + 1) % BUFF_LENGTH;
		pthread_mutex_lock(&file_data->buffers[buffer_id].lock);
		if (file_data->buffers[buffer_id].status != EMPTY)
		{
			minute_index = read_header(file_data->buffers[buffer_id].b_data, file_data, &buffer_index);
			read_data(&file_data->buffers[buffer_id], &buffer_index, &file_data->m_data[minute_index]);
			free(file_data->buffers[buffer_id].b_data);
			if (file_data->buffers[buffer_id].status == END)
				break ;
			file_data->buffers[buffer_id].status = EMPTY;
		}
		pthread_mutex_unlock(&file_data->buffers[buffer_id].lock);
	}
	for (i = 0; i < file_data->files_length; i++)
		save_second_file(&file_data->m_data[i]);
	log_message(LOG_INFO, "write_thread end");
	return (EXIT_SUCCESS);
}
