/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   session_log.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:35:29 by chulee            #+#    #+#             */
/*   Updated: 2023/04/18 17:34:42 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSION_LOG_H
# define SESSION_LOG_H
# define _XOPEN_SOURCE
# define EVENT_SIZE				(sizeof(struct inotify_event))
# define EVENT_BUFFER_SIZE		(1024 * (EVENT_SIZE + NAME_MAX + 1))
# define FILE_READ_BUFFER_SIZE	1024
# define HEADER_SIZE			sizeof(struct RawFileHeader2_t)
# define DATA_SIZE				sizeof(struct RawDataVer2_t)
# define MAX_CID_SIZE			5000
# define SECOND					60
# define BUFF_LENGTH			2
# include "raw_file_type.h"
# include "list.h"
# include "string_utils.h"
# include "log.h"
# include <time.h>
# include <stdlib.h>
# include <stdio.h>
# include <assert.h>
# include <string.h>
# include <fcntl.h>
# include <unistd.h>
# include <stdbool.h>
# include <bits/pthreadtypes.h>
# include <pthread.h>
# include <dirent.h>
# include <sys/inotify.h>
# include <limits.h>

enum	e_file_status
{
	NEW,
	END,
	EMPTY
};

struct second_data_byte
{
	unsigned long long	total_byte;
};

struct second_data
{
	struct second_data_byte	internal[MAX_CID_SIZE];
	struct second_data_byte	external[MAX_CID_SIZE];
};

struct minute_data
{
	struct tm			time_info;
	struct second_data	s_data[SECOND];
};

struct buffer
{
	pthread_mutex_t		lock;
	unsigned char		*b_data;
	int					read_size;
	enum e_file_status	status;
};

struct save_file
{
	List				*log_files;
	int					files_length;
	struct tm			start_tm;
	struct tm			end_tm;
	struct minute_data	*m_data;
	struct buffer		buffers[BUFF_LENGTH];
};

long	get_file_size(FILE *file);
void*	read_thread(void *arg);
void*	write_thread(void *arg);

#endif
