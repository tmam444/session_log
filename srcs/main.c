/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:56:45 by chulee            #+#    #+#             */
/*   Updated: 2023/04/12 10:03:55 by chulee           ###   ########.fr       */
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
		filename = make_filename(m_data->time_info, i);
		fp = fopen(filename, "w");
		for (j = 0; j < MAX_CID_SIZE; j++)
			if (m_data->data[i].internal[j].total_byte != 0)
				fprintf(fp, "%d,%llu,%llu\n", j, m_data->data[i].internal[j].total_byte, m_data->data[i].external[j].total_byte);
		free(filename);
		fclose(fp);
	}
}

void	save_data(struct RawDataVer2_t *data, struct minute_data *m_data)
{
	int	start_time, end_time, int_per_second_byte, ext_per_second_byte;

	start_time = data->start_time < 60 ? data->start_time : 0;
	end_time = data->end_time < 60 ? data->end_time : 60;
	int_per_second_byte = data->int_byte.byte;
	ext_per_second_byte = data->ext_byte.byte;
	if (end_time - start_time != 0)
	{
		int_per_second_byte /= end_time - start_time;
		ext_per_second_byte /= end_time - start_time;
	}
	while (start_time < end_time)
	{
		m_data->data[start_time].internal[data->int_cid].total_byte += int_per_second_byte;
		assert(m_data->data[start_time].internal[data->int_cid].total_byte >= (unsigned long long)int_per_second_byte);
		m_data->data[start_time].external[data->ext_cid].total_byte += ext_per_second_byte;
		assert(m_data->data[start_time].external[data->ext_cid].total_byte >= (unsigned long long)ext_per_second_byte);
		start_time++;
	}
}

void	setup(int argc, char *argv[], struct minute_data *m_data)
{
	int						read_size, i;
	FILE					*fp;
	struct RawFileHeader2_t	header;
	struct RawDataVer2_t	data;
	time_t					time;

	for (i = 1; i < argc; i++)
	{
		fp = fopen(argv[i], "r");
		assert(fp != NULL);
		read_size = fread(&header, HEADER_SIZE, 1, fp);
		if (read_size)
		{
			time = header.time;
			m_data->time_info = localtime(&time);
		}
		while ((read_size = fread(&data, DATA_SIZE, 1, fp)) > 0)
			save_data(&data, m_data);
		fclose(fp);
	}
}

int	main(int argc, char *argv[])
{
	struct minute_data	data;

	if (argc == 1)
	{
		fprintf(stderr, "Error!\nUsage : %s filename1 filename2 ...\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	memset(&data, 0, sizeof(struct minute_data));
	setup(argc, argv, &data);
	save_second_file(&data);
	return (EXIT_SUCCESS);
}
