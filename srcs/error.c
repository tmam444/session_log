/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/19 13:17:28 by chulee            #+#    #+#             */
/*   Updated: 2023/04/21 19:02:10 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"

static const char	*error_msg[] = {
	"Date parsing error",
	"Session directory not found",
	"Session log file not found",
	"File header time is not equals",
	"Session log file name is wrong",
	"Unknown Daemon Error, See Daemon Log",
	"Cmd data is wrong"
};

char*	make_error_filename(void)
{
	// const char	*path = "/usr/lib/qosd/tmp";
	const char	*path = "/home/chulee/session_log/temp";
	const int	cmd_number = 99;
	const int	filename_len = PATH_MAX;
	char		*filename;

	filename = malloc(filename_len);
	assert(filename != NULL);
	memset(filename, 0, filename_len);
	snprintf(filename, filename_len, "%s/real_result_%02d.error", path, cmd_number);
	return (filename);
}

void	create_error_file(error_code code)
{
	char	*filename;
	FILE		*fp;

	filename = make_error_filename();
	fp = fopen(filename, "w");
	fprintf(fp, "%d,%s\n", code, error_msg[code]);
	free(filename);
	fclose(fp);
	DEBUG_LOG("error! code = %d, msg = %s", code, error_msg[code]);
}
