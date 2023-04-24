/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/19 13:17:28 by chulee            #+#    #+#             */
/*   Updated: 2023/04/24 16:20:29 by chulee           ###   ########.fr       */
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

void	create_error_file(error_code code, int user_id)
{
	char	*filename;
	FILE	*fp;

	filename = make_real_filename(user_id);
	fp = fopen(filename, "w");
	fprintf(fp, "%d,%s\n", code, error_msg[code]);
	free(filename);
	fclose(fp);
	DEBUG_LOG("error! code = %d, msg = %s", code, error_msg[code]);
}
