/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/03 15:23:51 by chulee            #+#    #+#             */
/*   Updated: 2023/04/24 18:28:43 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "string_utils.h"

bool	ntk_str_ends_with(const char *haystack, const char *needle)
{
	bool	ret = false;
	char	*temp;

	temp = strstr(haystack, needle);
	if (temp)
	{
		temp += strlen(needle);
		if (*temp == '\0')
			ret = true;
	}
	return (ret);
}

char*	ntk_strdup(const char *s1)
{
    char	*ret;
    int		i;

    ret = malloc(strlen(s1) + 1);
	assert(ret != NULL);
    i = 0;
    while (s1[i] != '\0')
    {
        ret[i] = s1[i];
        i++;
    }
    ret[i] = '\0';
    return (ret);
}

char*	ntk_strndup(const char *s1, size_t str_length)
{
    char	*ret;
    size_t	i;

    ret = malloc(str_length + 1);
	assert(ret != NULL);
    i = 0;
	while (i < str_length)
	{
		ret[i] = s1[i];
		i++;
	}
	ret[i] = '\0';
	return (ret);
}

char*	ntk_strjoin(char const *s1, char const *s2)
{
	int		len;
	char	*str;

	if (!s1 || !s2)
		return (NULL);
	len = strlen(s1) + strlen(s2);
	str = malloc(len + 1);
	assert(str != NULL);
	strcpy(str, s1);
	strcat(str, s2);
	return (str);
}

char *ntk_strrstr(const char *haystack, const char *needle)
{
    int  haystack_len;
    int  needle_len;
    char *ptr;

    haystack_len = strlen(haystack);
    needle_len   = strlen(needle);

    if(needle_len == 0)
        return (char *)haystack;

    if(needle_len > haystack_len)
        return NULL;

    ptr = (char *)haystack + haystack_len - needle_len;
    
    while(1)
	{
        if(strncmp(ptr, needle, needle_len) == 0)
            return ptr;
        if(ptr == haystack)
            break;
		ptr--;
    }
    return NULL;
}
