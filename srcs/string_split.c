/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_split.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/11/14 20:32:02 by chulee            #+#    #+#             */
/*   Updated: 2023/04/18 11:48:21 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "string_utils.h"

static	int	count_words(char const *s, const char c)
{
	int	ret;
	int	flag;

	ret = 0;
	flag = 1;
	while (*s)
	{
		if (*s == c)
			flag = 1;
		else
		{
			if (flag)
			{
				ret++;
				flag = 0;
			}
		}
		s++;
	}
	return (ret);
}

static	int	find_start(char const *s, char c)
{
	int	i;

	i = 0;
	while (s[i] != '\0')
	{
		if (s[i] != c)
			break ;
		i++;
	}
	return (i);
}

static	int	find_len(char const *s, char c)
{
	int	i;

	i = 0;
	while (s[i] != '\0')
	{
		if (s[i] == c)
			break ;
		i++;
	}
	return (i);
}

static char    *substr(char const *s, unsigned int start, size_t len)
{
    char    *ret;
    size_t  i;

    if (!s)
		return (NULL);
    ret = malloc(len + 1);
    if (!ret)
		return (NULL);
    i = 0;
    while (start <= strlen(s) && i < len && s[i] != '\0')
		ret[i++] = s[start++];
    ret[i] = '\0';
    return (ret);
}

void	ntk_strsplit_free(char **str_splits)
{
	char	**temp;
	int		i;

	i = 0;
	if (str_splits)
	{
		temp = str_splits;
		while (temp[i] != NULL)
			free(temp[i++]);
		free(str_splits);
	}
}

char	**ntk_strsplit(char const *s, char delimiter)
{
	char	**ret;
	int		word_count;
	int		start;
	int		len;
	int		i;

	if (!s)
		return (NULL);
	word_count = count_words(s, delimiter);
	ret = malloc(sizeof(char *) * (word_count + 1));
	assert(ret != NULL);
	i = 0;
	start = 0;
	while (i < word_count)
	{
		start += find_start(s + start, delimiter);
		len = find_len(s + start, delimiter);
		ret[i] = substr(s, start, len);
		start += len;
		i++;
	}
	ret[i] = NULL;
	return (ret);
}
