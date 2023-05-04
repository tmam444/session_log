/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   program_setup.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/02 14:18:16 by chulee            #+#    #+#             */
/*   Updated: 2023/05/04 13:51:33 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "flow_simulator.h"

static void	program_exit(int signum)
{
	(void)signum;
	force_quit = true;
}

static void	log_file_setting(void)
{
	static const char*	log_file_path = "/var/log/flow_simulator.log";
	int					log_fd = open(log_file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);

    if (log_fd < 0)
	{
		perror("Error opening logfile");
		exit(EXIT_FAILURE);
    }
    if (dup2(log_fd, fileno(stdout)) < 0)
	{
		perror("Error redirecting stdout to logfile");
		exit(EXIT_FAILURE);
    }
}

void	program_setup(void)
{
	signal(SIGINT, program_exit);
	signal(SIGQUIT, program_exit);
	log_file_setting();
	(void)get_simulator();	// init simulator
}
