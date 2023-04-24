/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:56:45 by chulee            #+#    #+#             */
/*   Updated: 2023/04/24 18:29:43 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"

bool		force_quit;

static void	command_start(const char *cmd_directory_path, char *filename)
{
	pthread_t		cmd_tid;
	char			file_path[PATH_MAX];
	
    snprintf(file_path, sizeof(file_path), "%s/%s", cmd_directory_path, filename);
	pthread_create(&cmd_tid, NULL, command_thread, file_path);
	pthread_detach(cmd_tid);
}

static void	command_inotify(void)
{
	// const char				*cmd_directory_path = "/usr/lib/qosd/tmp";
	const char				*cmd_directory_path = "/home/chulee/session_log/command";
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
		if (length < 0)
		{
			if (!force_quit)
				log_message(LOG_WARNING, "Failed inotify_fd read");
			break ;
		}
		for (i = 0; i < length; i += EVENT_SIZE + event->len)
		{
			event = (struct inotify_event *)&buffer[i];
			if (!ntk_str_ends_with(event->name, "_tmp") && (event->mask & IN_CREATE || event->mask & IN_MOVED_TO))
				command_start(cmd_directory_path, event->name);
		}
    }
    inotify_rm_watch(inotify_fd, wd);
    close(inotify_fd);
}

static void	clear(void)
{
	struct session_simulator	*s_simulator = get_simulator();
	int							i;

	if (s_simulator)
	{
		for (i = 0; i < BUFF_LENGTH; i++)
			pthread_mutex_destroy(&s_simulator->buffers[i].lock);
		if (s_simulator->log_files)
			list_free(s_simulator->log_files);
		free(s_simulator);
	}
}

static void	program_exit(int signum)
{
	(void)signum;
	force_quit = true;
}

int	main(void)
{
	signal(SIGINT, program_exit);
	signal(SIGQUIT, program_exit);
	command_inotify();
	clear();
	return (EXIT_SUCCESS);
}
