/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   session_log.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:35:29 by chulee            #+#    #+#             */
/*   Updated: 2023/04/13 18:59:29 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSION_LOG_H
# define SESSION_LOG_H
# define HEADER_SIZE	sizeof(struct RawFileHeader2_t)
# define DATA_SIZE		sizeof(struct RawDataVer2_t)
# define MAX_CID_SIZE	5000
# define SECOND			60
# define BUFF_SIZE		4194304
# define BUFF_LENGTH	2
# include "raw_file_type.h"
# include <stdio.h>
# include <assert.h>
# include <string.h>
# include <time.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <unistd.h>
# include <stdbool.h>
# include <bits/pthreadtypes.h>
# include <arpa/inet.h>
# include <pthread.h>

enum	e_file_status
{
	NEW,
	CONTINUE,
	END
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
	unsigned char		b_data[BUFF_SIZE];
	bool				is_full;
	int					read_size;
	enum e_file_status	status;
};

struct save_file
{
	pthread_mutex_t		lock;
	char				**log_files;
	int					file_count;
	struct minute_data	*m_data;
	struct buffer		buffers[BUFF_LENGTH];
	bool				read_end;
};

#endif
