/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   write_thread.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 14:33:19 by chulee            #+#    #+#             */
/*   Updated: 2023/04/20 19:20:40 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"

char*	make_filename(int user_id)
{
	// const char	*path = "/usr/lib/qosd/tmp";
	const char	*path = "/home/chulee/session_log/temp";
	const int	cmd_number = 99;
	const int	filename_len = PATH_MAX;
	char		*filename;

	filename = malloc(filename_len);
	assert(filename != NULL);
	memset(filename, 0, filename_len);
	snprintf(filename, filename_len, "%s/real_result_%02d%02d", path, cmd_number, user_id);
	return (filename);
}

static void	save_second_file(struct minute_data *m_data, int second, int user_id)
{
	char		*filename;
	FILE		*fp;
	int			i;

	filename = make_filename(user_id);
	fp = fopen(filename, "w");
	printf("second = %d, user_id = %d\n", second, user_id);
	for (i = 0; i < MAX_CID_SIZE; i++)
		if (m_data->s_data[second].internal[i].total_byte != 0)
			fprintf(fp, "%d,%llu,%llu\n", i, m_data->s_data[second].internal[i].total_byte, m_data->s_data[second].external[i].total_byte);
	free(filename);
	fclose(fp);
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

static void	read_header(unsigned char *buff, struct session_simulator *s_simulator, int *buffer_index, \
						enum e_minute_index *m_index, error_code *err_code)
{
	struct RawFileHeader2_t	*header = (struct RawFileHeader2_t *)buff;
	time_t					header_time, s_time_minus_sec;

	assert(s_simulator != NULL && buff != NULL && buffer_index != NULL);
	s_time_minus_sec = s_simulator->stime - (s_simulator->stime % 60);
	header_time = header->time;
	if (header_time == s_time_minus_sec)
		*m_index = CUR;
	else if (header_time == s_time_minus_sec + 60)
		*m_index = NEXT;
	else
	{
		log_message(LOG_WARNING, "header time is not equals");
		*err_code = ERROR_FILE_HEADER;
		return ;
	}
	*buffer_index += HEADER_SIZE;
}

static void	read_data(struct buffer *buff, int *buffer_index, struct minute_data *m_data)
{
	int	i;

	for (i = *buffer_index; i < buff->read_size; i += DATA_SIZE)
		save_data(buff->b_data + i, m_data);
}

void*	write_thread(void *arg)
{
	struct session_simulator	*s_simulator = arg;
	bool						is_end = false;
	int							buffer_id, buffer_index;
	enum e_minute_index			m_index;
	error_code					*err_code;

	err_code = malloc(sizeof(error_code));
	assert(err_code != NULL);
	*err_code = NONE;
	buffer_id = 0;
	while (!is_end && *err_code == NONE)
	{
		buffer_index = 0;
		pthread_mutex_lock(&s_simulator->buffers[buffer_id].lock);
		if (s_simulator->buffers[buffer_id].status == NEW)
		{
			read_header(s_simulator->buffers[buffer_id].b_data, s_simulator, &buffer_index, &m_index, err_code);
			if (*err_code != NONE)
				read_data(&s_simulator->buffers[buffer_id], &buffer_index, &s_simulator->m_data[m_index]);
			free(s_simulator->buffers[buffer_id].b_data);
			s_simulator->buffers[buffer_id].status = EMPTY;
		}
		else if (s_simulator->buffers[buffer_id].status == END)
			is_end = true;
		pthread_mutex_unlock(&s_simulator->buffers[buffer_id].lock);
		buffer_id = (buffer_id + 1) % BUFF_LENGTH;
	}
	DEBUG_LOG("read data complete, file creation!");
	if (*err_code == NONE)
		save_second_file(&s_simulator->m_data[m_index], s_simulator->stime % 60, s_simulator->user_id);
	DEBUG_LOG("write thread end");
	return (err_code);
}
