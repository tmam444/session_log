# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: chulee <chulee@nstek.com>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/03/21 14:04:03 by chulee            #+#    #+#              #
#    Updated: 2023/04/13 18:45:25 by chulee           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME=session_simulator
SRCS=$(wildcard srcs/*.c)
OBJS=$(addprefix objs/, $(notdir $(SRCS:.c=.o)))
LOGS=$(wildcard log_file/*.txt)
CC=cc
INCLUDE=-I./includes 
CFLAGS=-O3 -Wall -Wextra -Werror
LDFLAGS=
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
