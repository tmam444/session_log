/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   write_thread.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 14:33:19 by chulee            #+#    #+#             */
/*   Updated: 2023/04/27 14:52:51 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "flow_simulator.h"

static void	read_header(unsigned char *buff, struct session_simulator *s_simulator,
						enum e_minute_index *m_index, error_code *err_code)
{
	struct RawFileHeader2_t	*header = (struct RawFileHeader2_t *)buff;
	time_t					header_time, s_time_minus_sec;

	assert(s_simulator != NULL && buff != NULL && m_index != NULL && err_code != NULL);
	s_time_minus_sec = s_simulator->stime - (s_simulator->stime % 60);
	header_time = header->time;
	if (header_time == s_time_minus_sec)
		*m_index = CUR;
	else if (header_time == s_time_minus_sec + 60)
		*m_index = NEXT;
	else
	{
		log_message(LOG_WARNING, "header time is not equals, s_time = %lld, header_time = %lld", s_time_minus_sec, header_time);
		*err_code = ERROR_FILE_HEADER;
	
		return ;
	}
}

static bool	check_validate_data(struct RawDataVer2_t *data, error_code *err_code)
{
	if (data->start_time > 128 || data->end_time > 128)
	{
		log_message(LOG_WARNING, "Raw Data Error, start_time : %d, end_time : %d", data->start_time, data->end_time);
		*err_code = ERROR_RAW_DATA;
		return (false);
	}
	else if (data->seg_num > MAX_SEG_SIZE)
	{
		log_message(LOG_WARNING, "Raw Data Error, seg_num : %d", data->seg_num);
		*err_code = ERROR_RAW_DATA;
		return (false);
	}
	else if (data->int_cid > MAX_CID_SIZE || data->ext_cid > MAX_CID_SIZE)
	{
		log_message(LOG_WARNING, "Raw Data Error, int_cid : %d, ext_cid : %d", data->int_cid, data->ext_cid);
		*err_code = ERROR_RAW_DATA;
		return (false);
	}
	return (true);
}

static void	read_data(struct buffer *buff, struct minute_data *m_data, error_code *err_code)
{
	struct RawDataVer2_t	*data;
	unsigned long long		start_time, end_time, int_per_second_byte, ext_per_second_byte, diff_time;
	int						i;

	for (i = HEADER_SIZE; i < buff->read_size; i += DATA_SIZE)
	{
		data = (struct RawDataVer2_t *)(buff->b_data + i);
		if (!check_validate_data(data, err_code))
			return;
		start_time = data->start_time < 60 ? data->start_time : 0;
		end_time = data->end_time < 60 ? data->end_time : 59;
		int_per_second_byte = data->int_byte.byte;
		ext_per_second_byte = data->ext_byte.byte;
		diff_time = end_time - start_time + 1;
		if (diff_time > 0)
		{
			if (int_per_second_byte > 0)
				int_per_second_byte = int_per_second_byte / diff_time;
			if (ext_per_second_byte > 0)
				ext_per_second_byte = ext_per_second_byte / diff_time;
		}
		while (start_time <= end_time)
		{
			m_data->s_data[data->seg_num][start_time].internal[data->int_cid].total_byte += int_per_second_byte;
			assert(m_data->s_data[data->seg_num][start_time].internal[data->int_cid].total_byte >= int_per_second_byte);
			m_data->s_data[data->seg_num][start_time].external[data->ext_cid].total_byte += ext_per_second_byte;
			assert(m_data->s_data[data->seg_num][start_time].external[data->ext_cid].total_byte >= ext_per_second_byte);
			m_data->t_data[data->seg_num][start_time].internal.total_byte += int_per_second_byte;
			assert(m_data->t_data[data->seg_num][start_time].internal.total_byte >= int_per_second_byte);
			m_data->t_data[data->seg_num][start_time].external.total_byte += ext_per_second_byte;
			assert(m_data->t_data[data->seg_num][start_time].internal.total_byte >= int_per_second_byte);
			start_time++;
		}
	}
}

void*	write_thread(void *__s_simulator)
{
	struct session_simulator	*s_simulator = __s_simulator;
	bool						is_end = false;
	int							buffer_id;
	enum e_minute_index			m_index;
	error_code					*err_code;

	err_code = malloc(sizeof(error_code));
	assert(err_code != NULL);
	*err_code = NONE;
	buffer_id = 0;
	while (!is_end && *err_code == NONE)
	{
		pthread_mutex_lock(&s_simulator->buffers[buffer_id].lock);
		if (s_simulator->buffers[buffer_id].status == NEW)
		{
			DEBUG_LOG("write_thread new file!");
			read_header(s_simulator->buffers[buffer_id].b_data, s_simulator, &m_index, err_code);
			if (*err_code == NONE)
				read_data(&s_simulator->buffers[buffer_id], &s_simulator->m_data[m_index], err_code);
			free(s_simulator->buffers[buffer_id].b_data);
			s_simulator->buffers[buffer_id].status = EMPTY;
		}
		else if (s_simulator->buffers[buffer_id].status == END)
			is_end = true;
		pthread_mutex_unlock(&s_simulator->buffers[buffer_id].lock);
		buffer_id = (buffer_id + 1) % BUFF_LENGTH;
	}
	DEBUG_LOG("write thread end");
	return (err_code);
}
