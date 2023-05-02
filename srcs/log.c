/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/18 13:43:27 by chulee            #+#    #+#             */
/*   Updated: 2023/05/02 10:55:54 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "log.h"

static const char	*LOG_LEVEL_NAMES[] = { "ERROR", "WARNING", "INFO", "DEBUG" };

void	log_message(log_level level, const char *format, ...)
{
	static pthread_mutex_t	log_mutex = PTHREAD_MUTEX_INITIALIZER;
    time_t					rawtime;
    struct tm				timeinfo;
    char					timestamp[20];

	pthread_mutex_lock(&log_mutex);
    time(&rawtime);
    localtime_r(&rawtime, &timeinfo);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

    printf("[%s] %s: ", timestamp, LOG_LEVEL_NAMES[level]);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
	pthread_mutex_unlock(&log_mutex);
	if (level == LOG_ERROR)
		exit(EXIT_FAILURE);
	fflush(stdout);
}
