/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:56:45 by chulee            #+#    #+#             */
/*   Updated: 2023/05/04 18:45:58 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "flow_simulator.h"

bool		force_quit;

static char*	make_file_path(const char *cmd_directory_path, const char *filename)
{
	char			*file_path;
	const int		str_len = PATH_MAX + NAME_MAX + 1;

	file_path = malloc(str_len);
	assert(file_path != NULL);
    snprintf(file_path, str_len, "%s/%s", cmd_directory_path, filename);
	return (file_path);
}

static char*	command_file_read(char *file_path, error_code *err_code)
{
	char		*file_path_rename = ntk_strjoin(file_path, "_tmp");
	char		*file_data;
	ssize_t		read_size, file_size;
	FILE		*fp;

	if (rename(file_path, file_path_rename) != EXIT_SUCCESS)
	{
		log_message(LOG_WARNING, "rename Failed, filename : %s", file_path);
		*err_code = ERROR_DAEMON;
		free(file_path_rename);
		return (NULL);
	}
	fp = fopen(file_path_rename, "r");
	if (fp == NULL)
	{
		log_message(LOG_WARNING, "cmd file open error, path : %s", file_path_rename);
		*err_code = ERROR_FILE_NOT_FOUND;
		free(file_path_rename);
		return (NULL);
	}
	file_size = get_file_size(fp);
	file_data = malloc(file_size + 1);
	assert(file_data != NULL);
	read_size = fread(file_data, file_size, 1, fp);
	if (read_size == 1)
		file_data[file_size] = '\0';
	else
		file_data[read_size] = '\0';
    fclose(fp);
	free(file_path_rename);
	DEBUG_LOG("command read complete");
	return (file_data);
}

static void	command_start(const char *cmd_directory_path, const char *filename)
{
	pthread_t		cmd_tid;
	error_code		err_code = NONE;
	char			*cmd_data;
	char			*file_path;
	char			**cmds;
	int				i;
	
	file_path = make_file_path(cmd_directory_path, filename);
	cmd_data = command_file_read(file_path, &err_code);
	free(file_path);
	if (err_code == NONE)
	{
		cmds = ntk_strsplit(cmd_data, '\n');
		free(cmd_data);
		i = 0;
		while (cmds[i] != NULL)
		{
			pthread_create(&cmd_tid, NULL, command_thread, ntk_strdup(cmds[i]));
			pthread_detach(cmd_tid);
			i++;
		}
		ntk_strsplit_free(cmds);
	}
}

static void	command_inotify(void)
{
	const char				*cmd_directory_path = "/usr/lib/qosd/tmp";
	const char				*cmd_prefix = "real_cmd_99";
    unsigned char			buffer[EVENT_BUFFER_SIZE];
	int						inotify_fd, wd, i, length;
	struct inotify_event	*event;

	inotify_fd = inotify_init();
    if (inotify_fd < 0)
		log_message(LOG_ERROR, "Failed inotify_init");
	log_message(LOG_INFO, "inotify start!");
    wd = inotify_add_watch(inotify_fd, cmd_directory_path, IN_CREATE | IN_MOVED_TO);
    if (wd < 0)
		log_message(LOG_ERROR, "Failed inotify_add_watch");

	force_quit = false;
    while (!force_quit)
	{
		length = read(inotify_fd, buffer, EVENT_BUFFER_SIZE);
		for (i = 0; i < length; i += EVENT_SIZE + event->len)
		{
			event = (struct inotify_event *)&buffer[i];
			if (!strncmp(event->name, cmd_prefix, strlen(cmd_prefix)) && !ntk_str_ends_with(event->name, "_tmp"))
				command_start(cmd_directory_path, event->name);
		}
    }
    inotify_rm_watch(inotify_fd, wd);
    close(inotify_fd);
}

int	main(void)
{
	program_setup();
	command_inotify();
	return (EXIT_SUCCESS);
}
