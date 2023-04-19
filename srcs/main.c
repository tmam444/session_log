/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:56:45 by chulee            #+#    #+#             */
/*   Updated: 2023/04/19 19:06:57 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"

bool	force_quit;

static bool	check_file_name(struct tm *search_tm, char *file_name)
{
	bool	ret = false;
	char	**tokens;
	int		file_hour, file_minute;
	int		token_length = 0;

	tokens = ntk_strsplit(file_name, '.');
	while (tokens[token_length] != NULL)
		token_length++;
	if (token_length == 4)
	{
		file_hour = atoi(tokens[2]);
		file_minute = atoi(tokens[3]);
		if (search_tm->tm_hour == file_hour && search_tm->tm_min + 1 == file_minute)
			ret = true;
	}
	ntk_strsplit_free(tokens);
	return (ret);
}

static void	find_files(struct session_simulator *s_simulator, struct tm *search_time)
{
	// const char		*path = "/var/qosd/log/raw";
	const char		*path = "/home/chulee/session_log/data";
	char			directory_path[512], file_path[1024];
	DIR				*dir;
    struct dirent	*entry;
	List			*temp, *new_log_files = NULL;

	snprintf(directory_path, sizeof(directory_path), "%s/%04d/%02d/%02d", \
				path, s_simulator->start_tm.tm_year + 1900, s_simulator->start_tm.tm_mon + 1, s_simulator->start_tm.tm_mday);
	dir = opendir(directory_path);
	if (dir == NULL)
	{
		log_message(LOG_WARNING, "Failed Directory Open - %s", directory_path);
		s_simulator->err_code = ERROR_DIRECTORY_NOT_FOUND;
		return;
	}
	while ((entry = readdir(dir)) != NULL)
	{
		if (check_file_name(search_time, entry->d_name))
		{
			snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);
			DEBUG_LOG("ADD File - %s", file_path);
			new_log_files = list_push(new_log_files, ntk_strdup(file_path));
			s_simulator->log_files = list_push(s_simulator->log_files, ntk_strdup(file_path));
			s_simulator->files_length++;
		}
	}
    closedir(dir);
}

static void	date_init(struct session_simulator *s_simulator, time_t start_time)
{
	time_t		end_time;

	localtime_r(&start_time, &s_simulator->start_tm);
	end_time = start_time + 60;
	localtime_r(&end_time, &s_simulator->end_tm);
}

void	update_time_and_remove_prev_data(struct session_simulator *s_simulator, time_t new_time)
{
	time_t	start_time;

	start_time = mktime(&s_simulator->start_tm) - s_simulator->start_tm.tm_sec;
	if (start_time <= new_time && new_time < start_time + 120)
	{
		if (new_time < start_time + 60)
			return ;
		else
		{
			memcpy(&s_simulator->m_data[CUR], &s_simulator->m_data[NEXT], sizeof(struct minute_data));
			memset(&s_simulator->m_data[NEXT], 0, sizeof(struct minute_data));
			date_init(s_simulator, new_time);
			find_files(s_simulator, &s_simulator->end_tm);
		}
	}
}

static void setup(struct session_simulator *s_simulator, struct command *command)
{
	const struct tm	empty_tm = {0};

	if (memcmp(&s_simulator->start_tm, &empty_tm, sizeof(struct tm)) == 0)
	{
		date_init(s_simulator, command->time);
		find_files(s_simulator, &s_simulator->start_tm);
		find_files(s_simulator, &s_simulator->end_tm);
	}
	else
	{
		update_time_and_remove_prev_data(s_simulator, command->time);
	}
}

static void	clear(struct session_simulator *s_simulator)
{
	int	i;

	if (s_simulator)
	{
		for (i = 0; i < BUFF_LENGTH; i++)
			pthread_mutex_destroy(&s_simulator->buffers[i].lock);
		if (s_simulator->log_files)
			list_free(s_simulator->log_files);
		free(s_simulator);
	}
}

struct session_simulator*	get_simulator(void)
{
	static struct session_simulator	*ret;
	int								i;

	if (ret == NULL)
	{
		ret = malloc(sizeof(struct session_simulator));
		assert(ret != NULL);
		memset(ret, 0, sizeof(struct session_simulator));
		for (i = 0; i < BUFF_LENGTH; i++)
		{
			ret->buffers[i].status = EMPTY;
			pthread_mutex_init(&ret->buffers[i].lock, NULL);
		}
		ret->m_data[CUR] = malloc(sizeof(struct minute_data));
		ret->m_data[NEXT] = malloc(sizeof(struct minute_data));
		ret->err_code = NONE;
	}
	return (ret);
}

void	command_do(struct command *command)
{
	pthread_t					read_thread_id, write_thread_id;
	struct session_simulator	*s_simulator;

	s_simulator = get_simulator();
	setup(s_simulator, command);
	if (s_simulator->files_length != 0)
	{
		DEBUG_LOG("thread init start!");
		pthread_create(&read_thread_id, NULL, read_thread, s_simulator);
		pthread_create(&write_thread_id, NULL, write_thread, s_simulator);
		pthread_join(read_thread_id, NULL);
		pthread_join(write_thread_id, NULL);
		DEBUG_LOG("thread join complete!");
	}
	if (s_simulator->err_code != NONE)
	{
		// save_error_file(log_data->err_code);
		s_simulator->err_code = NONE;
	}
	clear(s_simulator);
}

char*	command_read(char *file_path)
{
	char		*file_data;
	ssize_t		read_size, file_size;
	FILE		*fp;

	fp = fopen(file_path, "r");
	if (fp == NULL)
	{
		log_message(LOG_WARNING, "Failed File Open : %s\n", file_path);
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
	DEBUG_LOG("command read complete");
	return (file_data);
}

struct command*	command_init(char *file_data)
{
	int				i;
	int				*value;
	char			**tokens;
	struct command	*ret;

	ret = malloc(sizeof(struct command));
	assert(ret != NULL);
	tokens = ntk_strsplit(file_data, ',');
	ret->userid = atoi(tokens[0]);
	ret->time = atoi(tokens[1]);
	for (i = 2; tokens[i] != NULL; i++)
	{
		value = malloc(sizeof(int));
		*value = atoi(tokens[i]);
		ret->cid_list = list_push(ret->cid_list, value);
	}
	return (ret);
}

void	command_free(struct command *command)
{
	if (command)
	{
		if (command->cid_list)
			list_free(command->cid_list);
		free(command);
	}
}

void	command_check(char *directory_path, char *filename)
{
	char			file_path[PATH_MAX];
	char			*file_data;
	struct command	*command;

	DEBUG_LOG("command_check");
    snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, filename);
	file_data = command_read(file_path);
	if (file_data == NULL)
		return;
	command = command_init(file_data);
	command_do(command);
	command_free(command);
	free(file_data);
}

void	command_inotify(char *directory_path)
{
    unsigned char			buffer[EVENT_BUFFER_SIZE];
	int						inotify_fd, wd, i, length;
	struct inotify_event	*event;

	inotify_fd = inotify_init();
    if (inotify_fd < 0)
		log_message(LOG_ERROR, "Failed inotify_init");
	log_message(LOG_INFO, "inotify start!");
    wd = inotify_add_watch(inotify_fd, directory_path, IN_CREATE);
    if (wd < 0)
		log_message(LOG_ERROR, "Failed inotify_add_watch");
	force_quit = false;
    while (!force_quit) {
		length = read(inotify_fd, buffer, EVENT_BUFFER_SIZE);
		if (length < 0)
		{
			if (!force_quit)
				log_message(LOG_WARNING, "Failed inotify_fd read");
			break ;
		}
		i = 0;
		while (i < length) {
			event = (struct inotify_event *)&buffer[i];
			if (event->mask & IN_CREATE)
				command_check(directory_path, event->name);
			i += EVENT_SIZE + event->len;
		}
    }
    inotify_rm_watch(inotify_fd, wd);
    close(inotify_fd);
}

void	program_exit(int signum)
{
	(void)signum;
	force_quit = true;
}

int	main(int argc, char *argv[])
{
	if (argc != 2)
		log_message(LOG_ERROR, "Usage - %s directory_path", argv[0]);
	signal(SIGINT, program_exit);
	signal(SIGQUIT, program_exit);
	command_inotify(argv[1]);
	return (EXIT_SUCCESS);
}
