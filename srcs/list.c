/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   list.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/31 11:05:57 by chulee            #+#    #+#             */
/*   Updated: 2023/04/05 13:31:24 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "list.h"

static List*	list_create(void *value)
{
	List	*ret;

	ret = (List *)malloc(sizeof(List));
	assert(ret != NULL);
	ret->value = value;
	ret->next = NULL;
	return (ret);
}

List*	list_push(List *lst, void *value)
{
	List	*ret;

	ret = list_create(value);
	ret->next = lst;
	return (ret);
}

int		list_length(List *lst)
{
	int	ret = 0;

	while (lst != NULL)
	{
		ret++;
		lst = lst->next;
	}
	return (ret);
}

void	list_free(List *lst)
{
	List	*temp;

	while (lst != NULL)
	{
		if (lst->value)
			free(lst->value);
		temp = lst->next;
		free(lst);
		lst = temp;
	}
}

void*	list_search(List *lst, void *find_value, int (*cmp)(void *, void *))
{
	while (lst != NULL)
	{
		if (cmp(find_value, lst->value) == 0)
			return (lst->value);
		lst = lst->next;
	}
	return (NULL);
}

void	list_free_with_custom_free(List *lst, void (*value_free_function)(void *))
{
	List	*temp;

	assert(value_free_function != NULL);
	while (lst != NULL)
	{
		if (lst->value)
			value_free_function(lst->value);
		temp = lst->next;
		free(lst);
		lst = temp;
	}
}
