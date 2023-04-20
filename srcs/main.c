/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:56:45 by chulee            #+#    #+#             */
/*   Updated: 2023/04/20 19:15:39 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"

bool		force_quit;

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
		if (search_tm->tm_hour == file_hour && search_tm->tm_min == file_minute)
			ret = true;
	}
	ntk_strsplit_free(tokens);
	return (ret);
}

static void	find_session_files(struct session_simulator *s_simulator, const time_t search_time, error_code *err_code)
{
	// const char		*path = "/var/qosd/log/raw";
	const char		*path = "/home/chulee/session_log/data";
	struct tm		search_tm;
	char			directory_path[PATH_MAX], file_path[PATH_MAX * 2];
	DIR				*dir;
    struct dirent	*entry;

	localtime_r(&search_time, &search_tm);
	snprintf(directory_path, sizeof(directory_path), "%s/%04d/%02d/%02d", \
			path, search_tm.tm_year + 1900, search_tm.tm_mon + 1, search_tm.tm_mday);
	dir = opendir(directory_path);
	if (dir == NULL)
	{
		log_message(LOG_WARNING, "Failed Directory Open - %s", directory_path);
		*err_code = ERROR_DIRECTORY_NOT_FOUND;
		return;
	}
	while ((entry = readdir(dir)) != NULL)
	{
		if (check_file_name(&search_tm, entry->d_name))
		{
			snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);
			DEBUG_LOG("ADD File - %s", file_path);
			s_simulator->log_files = list_push(s_simulator->log_files, ntk_strdup(file_path));
			s_simulator->files_length++;
		}
	}
    closedir(dir);
}

void	update_time_and_find_session_files(struct session_simulator *s_simulator, struct command *command, error_code *err_code)
{
	time_t		s_time_minus_sec;

	s_time_minus_sec = s_simulator->stime - (s_simulator->stime % 60);
	s_simulator->stime = command->time;
	if (s_time_minus_sec <= command->time && command->time < s_time_minus_sec + 60)
	{
		DEBUG_LOG("EQUALS CUR MINUTE TIME");
	}
	else if (s_time_minus_sec - 60 <= command->time && command->time < s_time_minus_sec)
	{
		DEBUG_LOG("GET PREV MINUTE TIME FILE");
		memcpy(&s_simulator->m_data[NEXT], &s_simulator->m_data[CUR], sizeof(struct minute_data));
		memset(&s_simulator->m_data[CUR], 0, sizeof(struct minute_data));
		find_session_files(s_simulator, command->time - 60, err_code);
	}
	else if (s_time_minus_sec + 60 <= command->time && command->time < s_time_minus_sec + 120)
	{
		DEBUG_LOG("GET NEXT MINUTE TIME FILE");
		memcpy(&s_simulator->m_data[CUR], &s_simulator->m_data[NEXT], sizeof(struct minute_data));
		memset(&s_simulator->m_data[NEXT], 0, sizeof(struct minute_data));
		find_session_files(s_simulator, command->time + 60, err_code);
	}
	else
	{
		DEBUG_LOG("GET NEW MINUTE TIME FILE");
		find_session_files(s_simulator, command->time, err_code);
		if (*err_code == NONE)
			find_session_files(s_simulator, command->time + 60, err_code);
	}
}

static void setup(struct session_simulator *s_simulator, struct command *command, error_code *err_code)
{
	s_simulator->user_id = command->user_id;
	update_time_and_find_session_files(s_simulator, command, err_code);
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
	}
	return (ret);
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

void	command_do(struct command *command, error_code *err_code)
{
	pthread_t					read_thread_id, write_thread_id;
	struct session_simulator	*s_simulator;
	void						*read_retval = NULL;
	void						*write_retval = NULL;

	s_simulator = get_simulator();
	setup(s_simulator, command, err_code);
	if (*err_code == NONE && s_simulator->files_length != 0)
	{
		DEBUG_LOG("thread init start!");
		pthread_create(&read_thread_id, NULL, read_thread, s_simulator);
		pthread_create(&write_thread_id, NULL, write_thread, s_simulator);
		pthread_join(read_thread_id, &read_retval);
		pthread_join(write_thread_id, &write_retval);
		if (*err_code == NONE && *(error_code *)read_retval != NONE)
		{
			*err_code = *(error_code *)read_retval;
			free(read_retval);
		}
		if (*err_code == NONE && *(error_code *)write_retval != NONE)
		{
			*err_code = *(error_code *)write_retval;
			free(write_retval);
		}
		DEBUG_LOG("thread join complete!");
	}
	list_free(s_simulator->log_files);
	s_simulator->log_files = NULL;
	s_simulator->files_length = 0;
}

char*	command_read(char *file_path, error_code *err_code)
{
	char		*file_data;
	ssize_t		read_size, file_size;
	FILE		*fp;

	fp = fopen(file_path, "r");
	if (fp == NULL)
	{
		log_message(LOG_WARNING, "Failed File Open : %s", file_path);
		*err_code = ERROR_FILE_NOT_FOUND;
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

struct command*	command_init(char *file_data, error_code *err_code)
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
	if (token_size < 3)
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
			*value = atoi(tokens[i]);
			ret->cid_list = list_push(ret->cid_list, value);
		}
	}
	ntk_strsplit_free(tokens);
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

void	command_check(const char *cmd_directory_path, char *filename)
{
	char			file_path[PATH_MAX];
	char			*file_data;
	struct command	*command;
	error_code		err_code = NONE;

	DEBUG_LOG("command_check");
    snprintf(file_path, sizeof(file_path), "%s/%s", cmd_directory_path, filename);
	file_data = command_read(file_path, &err_code);
	if (err_code != NONE)
	{
		create_error_file(err_code);
		return;
	}
	command = command_init(file_data, &err_code);
	free(file_data);
	if (err_code != NONE)
	{
		create_error_file(err_code);
		return;
	}
	command_do(command, &err_code);
	command_free(command);
	if (err_code != NONE)
	{
		create_error_file(err_code);
		return;
	}
}

void	command_inotify(void)
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
    wd = inotify_add_watch(inotify_fd, cmd_directory_path, IN_CREATE);
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
				command_check(cmd_directory_path, event->name);
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

int	main(void)
{
	signal(SIGINT, program_exit);
	signal(SIGQUIT, program_exit);
	command_inotify();
	clear();
	return (EXIT_SUCCESS);
}
