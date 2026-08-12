CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -Iinclude

all:
	$(CC) $(CFLAGS) src/*.c -o powertools