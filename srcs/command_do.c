/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command_do.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 17:35:26 by chulee            #+#    #+#             */
/*   Updated: 2023/04/25 11:51:42 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"

static bool	check_file_name_equals_time(struct tm *search_tm, char *file_name)
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
		if (check_file_name_equals_time(&search_tm, entry->d_name))
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

static enum e_time	update_time(struct session_simulator *s_simulator, struct command *command)
{
	enum e_time	ret;
	time_t		s_time_minus_sec;

	s_time_minus_sec = s_simulator->stime - (s_simulator->stime % 60);
	s_simulator->stime = command->time;
	if (s_time_minus_sec - 60 <= command->time && command->time < s_time_minus_sec)
	{
		ret = PREV_TIME;
		s_simulator->is_cached = false;
		DEBUG_LOG("GET PREV MINUTE TIME FILE");
	}
	else if (s_time_minus_sec <= command->time && command->time < s_time_minus_sec + 60)
	{
		ret = CUR_TIME;
		s_simulator->is_cached = true;
		DEBUG_LOG("EQUALS CUR MINUTE TIME");
	}
	else if (s_time_minus_sec + 60 <= command->time && command->time < s_time_minus_sec + 120)
	{
		ret = NEXT_TIME;
		s_simulator->is_cached = true;
		DEBUG_LOG("GET NEXT MINUTE TIME FILE");
	}
	else
	{
		ret = ANOTHER_TIME;
		s_simulator->is_cached = false;
		DEBUG_LOG("GET NEW MINUTE TIME FILE");
	}
	return (ret);
}

static void	update_time_and_find_session_files(struct session_simulator *s_simulator, struct command *command, error_code *err_code)
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
	s_simulator->cmd = command;
	update_time_and_find_session_files(s_simulator, command, err_code);
}

static void	make_result_file(struct session_simulator *s_simulator)
{
	const struct minute_data	*m_data = &s_simulator->m_data[CUR];
	char						*temp_filename, *real_filename;
	FILE						*fp;
	int							i, second, *cid;
	List						*cur;

	temp_filename = make_temp_filename(s_simulator->cmd->user_id);
	second = s_simulator->stime % 60;
	fp = fopen(temp_filename, "w");
	fprintf(fp, "%ld", s_simulator->stime);

	//print Total, CID 0
	fprintf(fp, ",0");
	for (i = 0; i < MAX_SEG_SIZE; i++)
		fprintf(fp, ":%llu:%llu:%d:%d", m_data->t_data[i][second].internal.total_byte, \
										m_data->t_data[i][second].external.total_byte, 0, 0);
	for (cur = s_simulator->cmd->cid_list; cur != NULL; cur = cur->next)
	{
		cid = cur->value;
		fprintf(fp, ",%d", *cid);
		for (i = 0; i < MAX_SEG_SIZE; i++)
			fprintf(fp, ":%llu:%llu:%d:%d", m_data->s_data[i][second].internal[*cid].total_byte, \
											m_data->s_data[i][second].external[*cid].total_byte, 0, 0);
	}
	fclose(fp);
	real_filename = make_real_filename(s_simulator->cmd->user_id);
	rename(temp_filename, real_filename);
	free(real_filename);
	free(temp_filename);
}

static bool	command_check_is_running_and_set(struct session_simulator *s_simulator, const bool set_value)
{
	bool	prev_value;

	pthread_mutex_lock(&s_simulator->lock);
	prev_value = s_simulator->is_running;
	if (prev_value != set_value)
		s_simulator->is_running = set_value;
	pthread_mutex_unlock(&s_simulator->lock);
	return (prev_value);
}

static void	set_error_code(error_code *err_code, error_code *read_retval, error_code *write_retval)
{
	if (read_retval)
	{
		if (*read_retval != NONE && *err_code == NONE)
			*err_code = *read_retval;
		free(read_retval);
	}
	if (write_retval)
	{
		if (*write_retval != NONE && *err_code == NONE)
			*err_code = *write_retval;
		free(write_retval);
	}
}

void	command_do(struct command *command, error_code *err_code)
{
	struct session_simulator	*s_simulator;
	pthread_t					read_thread_id, write_thread_id;
	error_code					*read_retval = NULL, *write_retval = NULL;

	s_simulator = get_simulator();
	if (command_check_is_running_and_set(s_simulator, true))
	{
		*err_code = ERROR_CMD_IS_RUNNING;
		return ;
	}
	setup(s_simulator, command, err_code);
	if (s_simulator->is_cached && *err_code == NONE)
	{
		DEBUG_LOG("is cached..");
		make_result_file(s_simulator);
	}
	if (s_simulator->log_files && *err_code == NONE)
	{
		DEBUG_LOG("thread init start!");
		pthread_create(&read_thread_id, NULL, read_thread, s_simulator);
		pthread_create(&write_thread_id, NULL, write_thread, s_simulator);
		pthread_join(read_thread_id, (void **)&read_retval);
		pthread_join(write_thread_id, (void **)&write_retval);
		set_error_code(err_code, read_retval, write_retval);
		DEBUG_LOG("thread join complete!");
	}
	if (!s_simulator->is_cached && *err_code == NONE)
	{
		DEBUG_LOG("is not cached..");
		make_result_file(s_simulator);
	}
	list_free(s_simulator->log_files);
	s_simulator->log_files = NULL;
	command_check_is_running_and_set(s_simulator, false);
}
