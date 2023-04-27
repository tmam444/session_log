# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: chulee <chulee@nstek.com>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/03/21 14:04:03 by chulee            #+#    #+#              #
#    Updated: 2023/04/27 17:17:44 by chulee           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME=flow_simulator
SRCS=$(wildcard srcs/*.c)
OBJS=$(addprefix objs/, $(notdir $(SRCS:.c=.o)))
LOGS=$(wildcard log_file/*.txt)
CC=cc
INCLUDE=-I./includes 
CFLAGS=-fsanitize=address -g -Wall -Wextra -Werror
LDFLAGS=
OBJ_FILES=$(OBJS)
TEST_OBJS=$(filter-out objs/main.o, $(OBJS))
TEST_NAME=cunit_test

$(NAME) : $(OBJ_FILES)
		$(CC) $(CFLAGS) $(INCLUDE) -o $(NAME) $(OBJ_FILES) $(LDFLAGS)

objs/%.o : srcs/%.c
		$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@ $(LDFLAGS)

clean :
		rm -f $(OBJS)
		rm -f $(LOGS)
		rm -f $(TEST_NAME)

fclean : clean
		rm -f $(NAME)

test : $(OBJ_FILES)
		$(CC) $(CFLAGS) $(INCLUDE) -o $(TEST_NAME) test/cunit.c $(TEST_OBJS) -lcunit

all : $(NAME)

re : fclean all

.PHONY: clean fclean all re test
