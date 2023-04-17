/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:56:45 by chulee            #+#    #+#             */
/*   Updated: 2023/04/17 19:05:30 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "raw_file_type.h"
#include "session_log.h"
#include "string_utils.h"
#include <stdlib.h>
#include <time.h>

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

void	find_files(struct save_file *info)
{
	// const char		*path = "/var/qosd/log/raw";
	const char		*path = "/home/chulee/session_log/data";
	char			directory_path[512], file_path[1024];
	DIR				*dir;
    struct dirent	*entry;
    struct stat 	file_stat;
	time_t			start_t, end_t;

	snprintf(directory_path, sizeof(directory_path), "%s/%04d/%02d/%02d", path, info->start_tm.tm_year + 1900, info->start_tm.tm_mon + 1, info->start_tm.tm_mday);
	start_t = mktime(&info->start_tm);
	end_t = mktime(&info->end_tm);
	dir = opendir(directory_path);
	assert(dir != NULL);
	while ((entry = readdir(dir)) != NULL) {
		snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);
		if (stat(file_path, &file_stat) == -1) {
			perror("stat");
			continue;
		}
		if (S_ISREG(file_stat.st_mode)) {
			if (file_stat.st_ctime >= start_t && file_stat.st_ctime <= end_t)
			{
				info->log_files = list_push(info->log_files, ntk_strdup(file_path));
				info->files_length++;
			}
		}
	}
    closedir(dir);
}

void	date_parsing(char *start_time, struct save_file *info)
{
	time_t		end_t;
	struct tm	*end_tm, *t;

	if (strptime(start_time, "%Y-%m-%d-%H:%M", &info->start_tm) == NULL) {
		fprintf(stderr, "Error parsing date string : %s\n", start_time);
		exit(EXIT_FAILURE);
    }
	t = &info->start_tm;
	info->start_tm.tm_min += 2;
	printf("현재 시각: %d년 %d월 %d일 %d시 %d분 %d초\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
	end_t = mktime(&info->start_tm) + 120;
	end_tm = localtime(&end_t);
	memcpy(&info->end_tm, end_tm, sizeof(struct tm));
	t = &info->end_tm;
	printf("현재 시각: %d년 %d월 %d일 %d시 %d분 %d초\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
}

struct save_file*	setup(char *start_time, int argc, char *argv[])
{
	struct save_file		*ret;
	int						i;
	List					*cur;

	ret = malloc(sizeof(struct save_file));
	assert(ret != NULL);
	memset(ret, 0, sizeof(struct save_file));
	for (i = 0; i < BUFF_LENGTH; i++)
	{
		ret->buffers[i].status = EMPTY;
		pthread_mutex_init(&ret->buffers[i].lock, NULL);
	}
	(void)argc, (void)argv;
	pthread_mutex_init(&ret->lock, NULL);
	date_parsing(start_time, ret);
	find_files(ret);
	printf("length = %d\n", ret->files_length);
	for (cur = ret->log_files; cur != NULL; cur = cur->next)
		printf("files = %s\n", (char *)cur->value);
	ret->m_data = malloc(sizeof(struct minute_data) * (ret->files_length));
	memset(ret->m_data, 0, sizeof(struct minute_data) * (ret->files_length));
	return (ret);
}

long	get_file_size(FILE *file) {
	long current_position, file_size;

	current_position = ftell(file); // 현재 파일 포인터 위치를 저장합니다.
	fseek(file, 0, SEEK_END); // 파일 포인터를 파일의 끝으로 이동시킵니다.
    file_size = ftell(file); // 파일 포인터의 위치를 가져옵니다.
    fseek(file, current_position, SEEK_SET); // 파일 포인터를 원래 위치로 되돌립니다.
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
		{
			fprintf(stderr, "Error!\nFile Open Error : %s\n", (char *)cur->value);
			exit(EXIT_FAILURE);
		}
		read_size = -1;
		file_size = get_file_size(fp);
		printf("file_size = %d\n", file_size);
		buffer_id = (buffer_id + 1) % BUFF_LENGTH;
		pthread_mutex_lock(&file_data->buffers[buffer_id].lock);
		if (file_data->buffers[buffer_id].status == EMPTY)
		{
			file_data->buffers[buffer_id].b_data = malloc(file_size);
			assert(file_data->buffers[buffer_id].b_data != NULL);
			memset(file_data->buffers[buffer_id].b_data, 0, file_size);
			read_size = fread(file_data->buffers[buffer_id].b_data, 1, file_size, fp);
			printf("file_size = %d, read_size = %d, equals = %d\n", file_size, read_size, file_size == read_size);
			if (read_size == file_size)
				file_data->buffers[buffer_id].status = NEW;
			else
				file_data->buffers[buffer_id].status = CONTINUE;
			file_data->buffers[buffer_id].read_size = read_size;
		}
		pthread_mutex_unlock(&file_data->buffers[buffer_id].lock);
		fclose(fp);
		cur = cur->next;
	}
	pthread_mutex_lock(&file_data->lock);
	file_data->read_end = true;
	pthread_mutex_unlock(&file_data->lock);
	printf("read end!\n");
	return (EXIT_SUCCESS);
}

int	read_header(unsigned char *buff, struct save_file *file_info, int *buffer_index)
{
	struct RawFileHeader2_t	*header = (struct RawFileHeader2_t *)buff;
	struct tm				*t;
	int						minute_index;
	time_t					temp_time, start_time;
	
	assert(file_info != NULL && buff != NULL && buffer_index != NULL);
	start_time = mktime(&file_info->start_tm);
	temp_time = header->time;
	t = localtime(&temp_time);
	printf("현재 시각: %d년 %d월 %d일 %d시 %d분 %d초\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
	minute_index = ((temp_time - start_time) / 60) + 2;
	printf("minute_index = %d\n", minute_index);
	memcpy(&file_info->m_data[minute_index].time_info, t, sizeof(struct tm));
	*buffer_index += HEADER_SIZE;
	return (minute_index);
}

void	add_data(struct minute_data *m_data, void *data)
{
	struct RawDataVer2_t	*r_data = data;

	save_data(r_data, m_data);
}

void	read_data(struct buffer *buff, int *buffer_index, struct minute_data *m_data)
{
	static int					remaining_buffer_size;
	static unsigned char		remaining_data[DATA_SIZE];
	int							i, read_size;

	if (remaining_buffer_size != 0)
	{
		memcpy(remaining_data + DATA_SIZE - remaining_buffer_size, buff->b_data, remaining_buffer_size);
		add_data(m_data, remaining_data);
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
			add_data(m_data, buff->b_data + i);
	}
	remaining_buffer_size = i - buff->read_size;
}

void*	write_thread(void *arg)
{
	struct save_file		*file_data = arg;
	bool					is_end = false;
	int						minute_index, buffer_id, buffer_index;
	int						i;

	buffer_id = 0;
	while (!is_end)
	{
		buffer_index = 0;
		buffer_id = (buffer_id + 1) % BUFF_LENGTH;
		pthread_mutex_lock(&file_data->buffers[buffer_id].lock);
		if (file_data->buffers[buffer_id].status != EMPTY)
		{
			if (file_data->buffers[buffer_id].status == NEW)
				minute_index = read_header(file_data->buffers[buffer_id].b_data, file_data, &buffer_index);
			read_data(&file_data->buffers[buffer_id], &buffer_index, &file_data->m_data[minute_index]);
			if (file_data->buffers[buffer_id].status == END || file_data->buffers[buffer_id].status == NEW)
			{
				pthread_mutex_lock(&file_data->lock);
				is_end = file_data->read_end;
				pthread_mutex_unlock(&file_data->lock);
			}
			file_data->buffers[buffer_id].status = EMPTY;
			free(file_data->buffers[buffer_id].b_data);
			file_data->buffers[buffer_id].b_data = NULL;
			printf("free!\n");
		}
		pthread_mutex_unlock(&file_data->buffers[buffer_id].lock);
	}
	printf("write test!\n");
	for (i = 0; i < 2; i++)
		save_second_file(&file_data->m_data[i]);
	printf("write end!\n");
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
		if (file_data->log_files)
			list_free(file_data->log_files);
		free(file_data);
	}
}

int	main(int argc, char *argv[])
{
	pthread_t			read_thread_id, write_thread_id;
	struct save_file	*file_data;

	// if (argc != 2)
	if (argc < 2)
	{
		fprintf(stderr, "Error!\nUsage : %s YYYY-mm-DD-HH:MM[start_time]", argv[0]);
		exit(EXIT_FAILURE);
	}
	file_data = setup(argv[1], argc, argv);
	if (file_data->files_length != 0)
	{
		pthread_create(&read_thread_id, NULL, read_thread, file_data);
		pthread_create(&write_thread_id, NULL, write_thread, file_data);
		pthread_join(read_thread_id, NULL);
		pthread_join(write_thread_id, NULL);
	}
	clear(file_data);
	return (EXIT_SUCCESS);
}
