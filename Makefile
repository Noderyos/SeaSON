CC = gcc
CFLAGS ?= --std=c99 -Wall -Wextra -Werror -Iinclude

all: src/main.c src/season.c src/lexer.c
	$(CC) $(CFLAGS) -o main $^