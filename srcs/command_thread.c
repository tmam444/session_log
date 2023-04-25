/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command_thread.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 16:32:37 by chulee            #+#    #+#             */
/*   Updated: 2023/04/25 11:50:22 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"

static char*	command_file_read(char *file_path)
{
	char		*file_rename_path = ntk_strjoin(file_path, "_tmp");
	char		*file_data;
	ssize_t		read_size, file_size;
	FILE		*fp;

	rename(file_path, file_rename_path);
	fp = fopen(file_rename_path, "r");
	file_size = get_file_size(fp);
	file_data = malloc(file_size + 1);
	assert(file_data != NULL);
	read_size = fread(file_data, file_size, 1, fp);
	if (read_size == 1)
		file_data[file_size] = '\0';
	else
		file_data[read_size] = '\0';
    fclose(fp);
	free(file_rename_path);
	DEBUG_LOG("command read complete");
	return (file_data);
}

static struct command*	command_parsing(char *file_data, error_code *err_code)
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
		ret->user_id = atoi(tokens[0]);
		ret->time = atoi(tokens[1]);
		for (i = 2; tokens[i] != NULL; i++)
		{
			value = malloc(sizeof(int));
			assert(value != NULL);
			*value = atoi(tokens[i]);
			ret->cid_list = list_push_back(ret->cid_list, value);
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

void*	command_thread(void *file_path)
{
	char			*cmd_data;
	struct command	*command;
	error_code		err_code = NONE;

	DEBUG_LOG("command process");
	cmd_data = command_file_read(file_path);
	free(file_path);
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
