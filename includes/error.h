/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/19 11:37:57 by chulee            #+#    #+#             */
/*   Updated: 2023/04/20 18:54:18 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERROR_H
# define ERROR_H
# include <assert.h>
# include <stdlib.h>

typedef enum e_error {
	ERROR_DATE_PARSING,
	ERROR_DIRECTORY_NOT_FOUND,
	ERROR_FILE_NOT_FOUND,
	ERROR_FILE_HEADER,
	ERROR_FILE_NAME,
	ERROR_DAEMON,
	ERROR_CMD_DATA,
	NONE
} error_code;

void	create_error_file(error_code code);

#endif
