CC = gcc
CFLAGS ?= -Wall -Wextra -Werror -Iinclude

all: src/main.c src/season.c src/lexer.c
	$(CC) $(CFLAGS) -o main $^