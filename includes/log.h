/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/18 13:43:48 by chulee            #+#    #+#             */
/*   Updated: 2023/04/19 13:28:36 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOG_H
# define LOG_H
# include <stdio.h>
# include <stdarg.h>
# include <stdlib.h>
# include <stdbool.h>
# include <time.h>

// DEBUG 메시지 출력을 활성화하려면 이 줄의 주석을 제거하세요
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
    #define DEBUG_LOG(...) log_message(LOG_DEBUG, __VA_ARGS__)
#else
    #define DEBUG_LOG(...) (void)0
#endif

typedef enum {
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG
} log_level;

void log_message(log_level level, const char *format, ...);

#endif
