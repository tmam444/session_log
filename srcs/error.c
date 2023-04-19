/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/19 13:17:28 by chulee            #+#    #+#             */
/*   Updated: 2023/04/19 15:27:23 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "error.h"
#include "session_log.h"

static const char	*error_msg[] = {
	"Date Parsing Error",
	"Session Log File Not found",
	"File Header Time is not equals",
	"Unknown Daemon Error, See Daemon Log"
};


static void	save_error_file(error_code code, int user_id)
{
	char	*filename;

	filename = make_filename(user_id);
}

Error*	create_error(error_code code)
{
	Error				*ret;

	ret = malloc(sizeof(Error));
	assert(ret != NULL);
	ret->code = code;
	ret->message = error_msg[code];
	return (ret);
}
