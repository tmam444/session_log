# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: chulee <chulee@nstek.com>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/03/21 14:04:03 by chulee            #+#    #+#              #
#    Updated: 2023/04/11 18:20:15 by chulee           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME=session_log
SRCS=$(wildcard srcs/*.c)
OBJS=$(addprefix objs/, $(notdir $(SRCS:.c=.o)))
LOGS=$(wildcard log_file/*.txt)
CC=gcc
INCLUDE=-I./includes 
CFLAGS=-fsanitize=address -g#-Wall -Wextra -Werror
LDFLAGS=-lm
OBJ_FILES=$(OBJS)

$(NAME) : $(OBJ_FILES)
		$(CC) $(CFLAGS) $(INCLUDE) -o $(NAME) $(OBJ_FILES) $(LDFLAGS)

objs/%.o : srcs/%.c
		$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@ $(LDFLAGS)

clean :
		rm -f $(OBJS)
		rm -f $(LOGS)

fclean : clean
		rm -f $(NAME)

all : $(NAME)

re : fclean all

.PHONY: clean fclean all re
