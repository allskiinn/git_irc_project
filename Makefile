# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: gkiala <marvin@42.fr>                      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/04/11 14:14:45 by gkiala            #+#    #+#              #
#    Updated: 2026/04/11 14:14:47 by gkiala           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC_DIR = src/
INC_DIR = include/

SRCS = $(SRC_DIR)main.cpp \
       $(SRC_DIR)Server.cpp \
       $(SRC_DIR)Client.cpp \
       $(SRC_DIR)Channel.cpp \
       $(SRC_DIR)CommandHandler.cpp

OBJS = $(SRCS:.cpp=.o)

INCLUDES = -I$(INC_DIR)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
