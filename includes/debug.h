/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/05 16:00:24 by chulee            #+#    #+#             */
/*   Updated: 2023/04/05 16:30:49 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DEBUG_H
# define DEBUG_H
# include <time.h>
# include <stdio.h>

#define START_TIME(MSG) \
    printf("%s\n", MSG); \
    clock_t begin = clock();

#define END_TIME(MSG) \
    clock_t end = clock(); \
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC; \
    printf("%s : %f second\n", MSG, time_spent);

#endif
