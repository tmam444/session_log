/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:56:45 by chulee            #+#    #+#             */
/*   Updated: 2023/04/18 18:52:01 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "session_log.h"
#include "log.h"

bool	force_quit;

static bool	check_file_name(struct tm *start_tm, struct tm *end_tm, char *file_name)
{
	bool	ret = false;
	char	**tokens;
	int		file_hour, file_min;
	int		token_length = 0;

	tokens = ntk_strsplit(file_name, '.');
	while (tokens[token_length] != NULL)
		token_length++;
	if (token_length == 4)
	{
		file_hour = atoi(tokens[2]);
		file_min = atoi(tokens[3]);
		if (start_tm->tm_hour <= file_hour && start_tm->tm_min < file_min &&
			end_tm->tm_hour >= file_hour && end_tm->tm_min >= file_min)
			ret = true;
	}
	ntk_strsplit_free(tokens);
	return (ret);
}

static bool	find_files(struct save_file *info)
{
	// const char		*path = "/var/qosd/log/raw";
	const char		*path = "/home/chulee/session_log/data";
	char			directory_path[512], file_path[1024];
	DIR				*dir;
    struct dirent	*entry;

	snprintf(directory_path, sizeof(directory_path), "%s/%04d/%02d/%02d", path, info->start_tm.tm_year + 1900, info->start_tm.tm_mon + 1, info->start_tm.tm_mday);
	dir = opendir(directory_path);
	if (dir == NULL)
	{
		log_message(LOG_WARNING, "Directory Open - %s", directory_path);
		return (false);
	}
	while ((entry = readdir(dir)) != NULL) {
		snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);
		if (check_file_name(&info->start_tm, &info->end_tm, entry->d_name))
		{
			log_message(LOG_INFO, "ADD File - %s", file_path);
			info->log_files = list_push(info->log_files, ntk_strdup(file_path));
			info->files_length++;
		}
	}
    closedir(dir);
	return (true);
}

static bool	date_parsing(char *start_time, struct save_file *info)
{
	time_t		end_t;

	if (strptime(start_time, "%Y-%m-%d-%H:%M:%S", &info->start_tm) == NULL)
	{
		log_message(LOG_WARNING, "parsing date string - %s", start_time);
		return (false);
	}
	end_t = mktime(&info->start_tm) + 120;
	localtime_r(&end_t, &info->end_tm);
	return (true);
}

static struct save_file*	setup(char *start_time)
{
	struct save_file		*ret;
	int						i;

	ret = malloc(sizeof(struct save_file));
	assert(ret != NULL);
	memset(ret, 0, sizeof(struct save_file));
	for (i = 0; i < BUFF_LENGTH; i++)
	{
		ret->buffers[i].status = EMPTY;
		pthread_mutex_init(&ret->buffers[i].lock, NULL);
	}
	if (!date_parsing(start_time, ret))
		return (ret);
	if (!find_files(ret))
		return (ret);
	if (ret->files_length != 0)
		ret->files_length = 2;
	ret->m_data = malloc(sizeof(struct minute_data) * (ret->files_length));
	memset(ret->m_data, 0, sizeof(struct minute_data) * (ret->files_length));
	return (ret);
}

static void	clear(struct save_file *file_data)
{
	int	i;

	if (file_data)
	{
		if (file_data->m_data)
			free(file_data->m_data);
		for (i = 0; i < BUFF_LENGTH; i++)
			pthread_mutex_destroy(&file_data->buffers[i].lock);
		if (file_data->log_files)
			list_free(file_data->log_files);
		free(file_data);
	}
}

void	command_do(char *command)
{
	pthread_t			read_thread_id, write_thread_id;
	struct save_file	*file_data;

	file_data = setup(command);
	if (file_data->files_length != 0)
	{
		log_message(LOG_INFO, "thread init start!");
		pthread_create(&read_thread_id, NULL, read_thread, file_data);
		pthread_create(&write_thread_id, NULL, write_thread, file_data);
		pthread_join(read_thread_id, NULL);
		pthread_join(write_thread_id, NULL);
		log_message(LOG_INFO, "thread join complete");
	}
	clear(file_data);
}

char*	command_read(char *file_path)
{
	char		*file_data;
	ssize_t		read_size, file_size;
	FILE		*fp;

	fp = fopen(file_path, "r");
	if (fp == NULL)
	{
		log_message(LOG_WARNING, "File Open : %s\n", file_path);
		return (NULL);
	}
	file_size = get_file_size(fp);
	file_data = malloc(file_size + 1);
	read_size = fread(file_data, file_size, 1, fp);
	if (read_size == 1)
		file_data[file_size] = '\0';
	else
		file_data[read_size] = '\0';
    fclose(fp);
	log_message(LOG_INFO, "command read complete");
	return (file_data);
}

char*	command_parsing(char *file_data)
{
	return (file_data);
}

void	command_check(char *directory_path, char *filename)
{
	char			file_path[PATH_MAX];
	char			*file_data;
	char			*command;

	log_message(LOG_INFO, "test~~");
    snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, filename);
	file_data = command_read(file_path);
	if (file_data == NULL)
		return;
	command = command_parsing(file_data);
	command_do(command);
	free(file_data);
}

void	command_inotify(char *directory_path)
{
    unsigned char			buffer[EVENT_BUFFER_SIZE];
	int						inotify_fd, wd, i, length;
	struct inotify_event	*event;

	inotify_fd = inotify_init();
    if (inotify_fd < 0)
		log_message(LOG_ERROR, "inotify_init failed");
	log_message(LOG_INFO, "inotify start!");
    wd = inotify_add_watch(inotify_fd, directory_path, IN_CREATE);
    if (wd < 0)
		log_message(LOG_ERROR, "inotify_add_watch failed");
	force_quit = false;
    while (!force_quit) {
		length = read(inotify_fd, buffer, EVENT_BUFFER_SIZE);
		if (length < 0)
		{
			log_message(LOG_WARNING, "inotify_fd read failed");
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
