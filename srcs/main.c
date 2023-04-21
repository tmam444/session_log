/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:56:45 by chulee            #+#    #+#             */
/*   Updated: 2023/04/21 18:31:33 by chulee           ###   ########.fr       */
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
	bool			check_find = false;

	DEBUG_LOG("search_time = %lld", search_time);
	localtime_r(&search_time, &search_tm);
	printf("%04d-%02d-%02d %02d:%02d:%02d\n",
         search_tm.tm_year + 1900, search_tm.tm_mon + 1, search_tm.tm_mday,
         search_tm.tm_hour, search_tm.tm_min, search_tm.tm_sec);
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
			check_find = true;
			snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);
			DEBUG_LOG("ADD File - %s", file_path);
			s_simulator->log_files = list_push_back(s_simulator->log_files, ntk_strdup(file_path));
		}
	}
	closedir(dir);
	if (check_find == false)
	{
		log_message(LOG_WARNING, "Failed Found Session File, filename = %d.%d", search_tm.tm_hour, search_tm.tm_min);
		*err_code = ERROR_FILE_NOT_FOUND;
	}
}

enum e_time	update_time(struct session_simulator *s_simulator, struct command *command)
{
	enum e_time	ret;
	time_t		s_time_minus_sec;

	s_time_minus_sec = s_simulator->stime - (s_simulator->stime % 60);
	s_simulator->stime = command->time;
	if (s_time_minus_sec - 60 <= command->time && command->time < s_time_minus_sec)
	{
		ret = PREV_TIME;
		s_simulator->now_cached = false;
		DEBUG_LOG("GET PREV MINUTE TIME FILE");
	}
	else if (s_time_minus_sec <= command->time && command->time < s_time_minus_sec + 60)
	{
		ret = CUR_TIME;
		s_simulator->now_cached = true;
		DEBUG_LOG("EQUALS CUR MINUTE TIME");
	}
	else if (s_time_minus_sec + 60 <= command->time && command->time < s_time_minus_sec + 120)
	{
		ret = NEXT_TIME;
		s_simulator->now_cached = true;
		DEBUG_LOG("GET NEXT MINUTE TIME FILE");
	}
	else
	{
		ret = ANOTHER_TIME;
		s_simulator->now_cached = false;
		DEBUG_LOG("GET NEW MINUTE TIME FILE");
	}
	return (ret);
}

void	update_time_and_find_session_files(struct session_simulator *s_simulator, struct command *command, error_code *err_code)
{
	enum e_time	check_cur_time;

	check_cur_time = update_time(s_simulator, command);
	if (check_cur_time == PREV_TIME)
	{
		memcpy(&s_simulator->m_data[NEXT], &s_simulator->m_data[CUR], sizeof(struct minute_data));
		memset(&s_simulator->m_data[CUR], 0, sizeof(struct minute_data));
		find_session_files(s_simulator, command->time, err_code);
	}
	else if (check_cur_time == CUR_TIME)
		return ;
	else if (check_cur_time == NEXT_TIME)
	{
		memcpy(&s_simulator->m_data[CUR], &s_simulator->m_data[NEXT], sizeof(struct minute_data));
		memset(&s_simulator->m_data[NEXT], 0, sizeof(struct minute_data));
		find_session_files(s_simulator, command->time + 120, err_code);
	}
	else
	{
		find_session_files(s_simulator, command->time + 60, err_code);
		if (*err_code == NONE)
			find_session_files(s_simulator, command->time + 120, err_code);
	}
	if (*err_code != NONE)
		s_simulator->stime = 0;
}

static void setup(struct session_simulator *s_simulator, struct command *command, error_code *err_code)
{
	s_simulator->cid_list = command->cid_list;
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

static void	save_result_file(struct session_simulator *s_simulator)
{
	const struct minute_data	*m_data = &s_simulator->m_data[CUR];
	char						*temp_filename, *real_filename;
	FILE						*fp;
	int							i, second, *cid;
	List						*cur;

	temp_filename = make_temp_filename(s_simulator->user_id);
	second = s_simulator->stime % 60;
	fp = fopen(temp_filename, "w");
	fprintf(fp, "%ld", s_simulator->stime);

	//print Total, CID 0
	fprintf(fp, ",0");
	for (i = 0; i < MAX_SEG_SIZE; i++)
		fprintf(fp, ":%llu:%llu:%d:%d", m_data->t_data[i][second].internal.total_byte, \
										m_data->t_data[i][second].external.total_byte, 0, 0);
	for (cur = s_simulator->cid_list; cur != NULL; cur = cur->next)
	{
		cid = cur->value;
		fprintf(fp, ",%d", *cid);
		for (i = 0; i < MAX_SEG_SIZE; i++)
			fprintf(fp, ":%llu:%llu:%d:%d", m_data->s_data[i][second].internal[*cid].total_byte, \
											m_data->s_data[i][second].external[*cid].total_byte, 0, 0);
	}
	fclose(fp);
	real_filename = make_real_filename(s_simulator->user_id);
	rename(temp_filename, real_filename);
	free(real_filename);
	free(temp_filename);
}

void	command_do(struct command *command, error_code *err_code)
{
	struct session_simulator	*s_simulator;
	pthread_t					read_thread_id, write_thread_id;
	void						*read_retval = NULL;
	void						*write_retval = NULL;

	s_simulator = get_simulator();
	setup(s_simulator, command, err_code);
	if (*err_code == NONE && s_simulator->now_cached)
	{
		DEBUG_LOG("is cached..");
		save_result_file(s_simulator);
	}
	if (*err_code == NONE && s_simulator->log_files != NULL)
	{
		DEBUG_LOG("thread init start!");
		pthread_create(&read_thread_id, NULL, read_thread, s_simulator);
		pthread_create(&write_thread_id, NULL, write_thread, s_simulator);
		pthread_join(read_thread_id, &read_retval);
		pthread_join(write_thread_id, &write_retval);
		if (read_retval)
		{
			if (*err_code == NONE && *(error_code *)read_retval != NONE)
				*err_code = *(error_code *)read_retval;
			free(read_retval);
		}
		if (write_retval)
		{
			if (*err_code == NONE && *(error_code *)write_retval != NONE)
				*err_code = *(error_code *)write_retval;
			free(write_retval);
		}
		DEBUG_LOG("thread join complete!");
	}
	if (*err_code == NONE && s_simulator->now_cached == false)
	{
		DEBUG_LOG("is not cached..");
		save_result_file(s_simulator);
	}
	list_free(s_simulator->log_files);
	s_simulator->log_files = NULL;
}

char*	command_read(char *file_path)
{
	char		*file_data;
	ssize_t		read_size, file_size;
	FILE		*fp;

	fp = fopen(file_path, "r");
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
			*value = atoi(tokens[i]);
			ret->cid_list = list_push_back(ret->cid_list, value);
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
	file_data = command_read(file_path);
	command = command_init(file_data, &err_code);
	free(file_data);
	if (err_code != NONE)
	{
		create_error_file(err_code);
		command_free(command);
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
    while (!force_quit)
	{
		length = read(inotify_fd, buffer, EVENT_BUFFER_SIZE);
		if (length < 0)
		{
			if (!force_quit)
				log_message(LOG_WARNING, "Failed inotify_fd read");
			break ;
		}
		i = 0;
		while (i < length)
		{
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
