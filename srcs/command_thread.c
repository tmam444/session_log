/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command_thread.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 16:32:37 by chulee            #+#    #+#             */
/*   Updated: 2023/04/27 14:43:49 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "flow_simulator.h"

struct command*	command_parsing(char *file_data, error_code *err_code)
{
	int				i, token_size = 0;
	int				*value;
	char			**tokens;
	struct command	*ret;

	DEBUG_LOG("file_data = %s", file_data);
	ret = malloc(sizeof(struct command));
	memset(ret, 0, sizeof(struct command));
	assert(ret != NULL);
	tokens = ntk_strsplit(file_data, ',');
	while (tokens[token_size] != NULL)
		token_size++;
	DEBUG_LOG("token size = %d", token_size);
	if (token_size < 3 || tokens[2][0] == '\0' || tokens[2][0] == '\n')
	{
		log_message(LOG_WARNING, "cmd file data error, data : %s", file_data);
		*err_code = ERROR_CMD_DATA;
	}
	else
	{
		ret->user_id = atoi(tokens[0] + 1); //remove #, #10 -> 10
		ret->time = atoi(tokens[1]);
		for (i = 2; tokens[i] != NULL; i++)
		{
			value = malloc(sizeof(int));
			assert(value != NULL);
			*value = atoi(tokens[i]);
			ret->cid_list = list_push_back(ret->cid_list, value);
			if (*value >= MAX_CID_SIZE)
			{
				log_message(LOG_WARNING, "cmd file CID data error, cid : %d", *value);
				*err_code = ERROR_CMD_DATA;
			}
		}
		if (ret->time == 0)
		{
			log_message(LOG_WARNING, "cmd file time data error, time : %d", ret->time);
			*err_code = ERROR_CMD_DATA;
		}
	}
	ntk_strsplit_free(tokens);
	return (ret);
}

static void	command_free(struct command *command)
{
	if (command)
	{
		if (command->cid_list)
			list_free(command->cid_list);
		free(command);
	}
}

void*	command_thread(void *cmd_data)
{
	struct command	*command;
	error_code		err_code = NONE;

	DEBUG_LOG("command process");
	command = command_parsing(cmd_data, &err_code);
	free(cmd_data);
	if (err_code != NONE)
	{
		command_free(command);
		pthread_exit(NULL);
	}
	command_do(command, &err_code);
	if (err_code != NONE)
		create_error_file(err_code, command->user_id);
	command_free(command);
	DEBUG_LOG("command process end!");
	return (EXIT_SUCCESS);
}
