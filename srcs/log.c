/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/18 13:43:27 by chulee            #+#    #+#             */
/*   Updated: 2023/04/18 14:16:36 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "log.h"
#include <time.h>

void log_message(log_level level, const char *format, ...) {
	static const char	*LOG_LEVEL_NAMES[] = { "ERROR", "WARNING", "INFO", "DEBUG" };
    time_t				rawtime;
    struct tm			*timeinfo;
    char				timestamp[20];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    printf("[%s] %s: ", timestamp, LOG_LEVEL_NAMES[level]);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
	if (level == LOG_ERROR)
		exit(EXIT_FAILURE);
}
