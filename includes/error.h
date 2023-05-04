/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/19 11:37:57 by chulee            #+#    #+#             */
/*   Updated: 2023/05/04 17:11:12 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERROR_H
# define ERROR_H
# include <assert.h>
# include <stdlib.h>

typedef enum e_error {
	ERROR_DIRECTORY_NOT_FOUND,
	ERROR_FILE_NOT_FOUND,
	ERROR_FILE_HEADER,
	ERROR_DAEMON,
	ERROR_CMD_DATA,
	ERROR_CMD_IS_RUNNING,
	ERROR_RAW_DATA,
	ERROR_SCRIPT_RUN_FAILED,
	ERROR_SCRIPT_RESULT_FILE_OPEN_FAILED,
	NONE
} error_code;

void	create_error_file(error_code code, int user_id);

#endif
