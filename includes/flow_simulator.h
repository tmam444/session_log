/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   flow_simulator.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:35:29 by chulee            #+#    #+#             */
/*   Updated: 2023/04/27 11:06:07 by chulee           ###   ########.fr       */
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
# define MAX_SEG_SIZE			8
# define MAX_CID_SIZE			5000
# define SECOND					60
# define BUFF_LENGTH			2
# define BUFF_MINUTE			2
# define ERROR_FILE_NUMBER		999
# include "raw_file_type.h"
# include "list.h"
# include "string_utils.h"
# include "log.h"
# include "error.h"
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
# include <signal.h>

enum	e_file_status
{
	NEW,
	END,
	EMPTY
};

enum	e_minute_index
{
	CUR,
	NEXT
};

enum	e_time
{
	PREV_TIME,
	CUR_TIME,
	NEXT_TIME,
	ANOTHER_TIME
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

struct total_data
{
	struct second_data_byte	internal;
	struct second_data_byte	external;
};

struct minute_data
{
	struct second_data		s_data[MAX_SEG_SIZE][SECOND];
	struct total_data		t_data[MAX_SEG_SIZE][SECOND];
};

struct buffer
{
	pthread_mutex_t		lock;
	unsigned char		*b_data;
	int					read_size;
	enum e_file_status	status;
};

struct command
{
	int		user_id;
	time_t	time;
	List	*cid_list;
};

struct session_simulator
{
	pthread_mutex_t		lock;
	List				*log_files;
	time_t				stime;
	struct minute_data	m_data[BUFF_MINUTE];
	struct buffer		buffers[BUFF_LENGTH];
	bool				is_cached;
	bool				is_running;
	struct command		*cmd;
};

struct session_simulator*	get_simulator(void);
long						get_file_size(FILE *file);
void						command_do(struct command *command, error_code *err_code);
char*						command_file_read(char *file_path, error_code *err_code);
struct command*				command_parsing(char *file_data, error_code *err_code);
void*						command_thread(void *file_path);
void*						read_thread(void *__s_simulator);
void*						write_thread(void *__s_simulator);
char*						make_real_filename(int user_id);
char*						make_temp_filename(int user_id);
char*						make_error_filename(int user_id);

extern bool			force_quit;

#endif
