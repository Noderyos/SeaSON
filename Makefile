CC = gcc
CFLAGS ?= --std=c99 -Wall -Wextra -Werror -Iinclude

all: src/main.c
	$(CC) $(CFLAGS) -o main $^