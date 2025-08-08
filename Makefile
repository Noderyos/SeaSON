CC = gcc
CFLAGS ?= -Wall -Wextra -Werror -Iinclude

all: src/main.c src/season.c
	$(CC) $(CFLAGS) -o main $^