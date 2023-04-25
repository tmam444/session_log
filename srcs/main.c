/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:56:45 by chulee            #+#    #+#             */
/*   Updated: 2023/04/25 18:59:23 by chulee           ###   ########.fr       */
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

void	print_data(struct RawDataVer2_t *data)
{
	char					int_ip_str[INET_ADDRSTRLEN];
	char					ext_ip_str[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, (void *)&data->int_ipv4, int_ip_str, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, (void *)&data->ext_ipv4, ext_ip_str, INET_ADDRSTRLEN);
	printf("\nint ip = %s\n", int_ip_str);
	printf("ext ip = %s\n", ext_ip_str);
	printf("seg_num = %d\n", data->seg_num);
	printf("protocol = %d\n", data->protocol);
	printf("int_cid = %d, ext_cid = %d\n", data->int_cid, data->ext_cid);
	printf("int_byte = %llu, ext_byte = %llu\n", (unsigned long long)data->int_byte.byte, (unsigned long long)data->ext_byte.byte);
	printf("start_time = %d, end_time = %d\n", data->start_time, data->end_time);
}

void	save_data(struct RawDataVer2_t *data, struct minute_data *m_data)
{
	unsigned long long	start_time, end_time, int_per_second_byte, ext_per_second_byte;

	start_time = data->start_time < 60 ? data->start_time : 0;
	end_time = data->end_time < 60 ? data->end_time : 59;
	int_per_second_byte = data->int_byte.byte;
	ext_per_second_byte = data->ext_byte.byte;
	printf("int_per_second_byte = %llu, ext_per_second_byte = %llu\n", int_per_second_byte, ext_per_second_byte);
	int_per_second_byte /= (end_time - start_time + 1);
	ext_per_second_byte /= (end_time - start_time + 1);
	printf("int_per_second_byte = %llu, ext_per_second_byte = %llu\n", int_per_second_byte, ext_per_second_byte);
	while (start_time <= end_time)
	{
		m_data->data[start_time].internal[data->int_cid].total_byte += int_per_second_byte;
		assert(m_data->data[start_time].internal[data->int_cid].total_byte >= int_per_second_byte);
		m_data->data[start_time].external[data->ext_cid].total_byte += ext_per_second_byte;
		assert(m_data->data[start_time].external[data->ext_cid].total_byte >= ext_per_second_byte);
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
		memset(m_data, 0, sizeof(struct minute_data));
		memset(&header, 0, sizeof(struct RawFileHeader2_t));
		memset(&data, 0, sizeof(struct RawDataVer2_t));
		fp = fopen(argv[i], "r");
		assert(fp != NULL);
		read_size = fread(&header, HEADER_SIZE, 1, fp);
		if (read_size)
		{
			time = header.time;
			m_data->time_info = localtime(&time);
			if (m_data->time_info == NULL)
			{
				fprintf(stderr, "Header Parsing Error!\n");
				exit(EXIT_FAILURE);
			}
		}
		while ((read_size = fread(&data, DATA_SIZE, 1, fp)) > 0)
		{
			print_data(&data);
			save_data(&data, m_data);
		}
		save_second_file(m_data);
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
	setup(argc, argv, &data);
	return (EXIT_SUCCESS);
}
