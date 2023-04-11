/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   session_log.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:35:29 by chulee            #+#    #+#             */
/*   Updated: 2023/04/11 18:01:22 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSION_LOG_H
# define SESSION_LOG_H
# define HEADER_SIZE	9
# define DATA_SIZE		126
# define MAX_CID_SIZE	5000
# define SECOND			60
# include "raw_file_type.h"
# include "string_utils.h"
# include <stdio.h>
# include <fcntl.h>
# include <string.h>

struct second_data_byte {
	unsigned long long	total_byte;
};

struct second_data {
	struct second_data_byte	internal[MAX_CID_SIZE];
	struct second_data_byte	external[MAX_CID_SIZE];
};

struct minute_data {
	struct tm			*time_info;
	struct second_data	data[SECOND];
};

#endif
