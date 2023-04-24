/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   make_filename.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 16:38:52 by chulee            #+#    #+#             */
/*   Updated: 2023/04/24 18:59:21 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"

char*	make_real_filename(int user_id)
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

char*	make_temp_filename(int user_id)
{
	// const char	*path = "/usr/lib/qosd/tmp";
	const char	*path = "/home/chulee/session_log/temp";
	const int	cmd_number = 99;
	const int	filename_len = PATH_MAX;
	char		*filename;

	filename = malloc(filename_len);
	assert(filename != NULL);
	memset(filename, 0, filename_len);
	snprintf(filename, filename_len, "%s/real_result_%02d%02d_tmp", path, cmd_number, user_id);
	return (filename);
}
