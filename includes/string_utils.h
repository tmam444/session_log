/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_utils.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:52:08 by chulee            #+#    #+#             */
/*   Updated: 2023/04/14 18:42:28 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STRING_UTILS_H
# define STRING_UTILS_H
# include <stdlib.h>
# include <string.h>
# include <assert.h>

void    ntk_strsplit_free(char **str_splits);
char**  ntk_strsplit(char const *s, char delimiter);
char*   ntk_strjoin(char const *s1, char const *s2);
char*   ntk_strdup(const char *s1);
char*   ntk_strndup(const char *s1, size_t str_length);
char*   ntk_strrstr(const char *haystack, const char *needle);
char*   ntk_itoa(int n);

#endif
