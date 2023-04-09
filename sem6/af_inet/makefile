CC := gcc
CFLAGS := -std=c99

.PHONY: clean

all: server client

clean:
	rm server client

server client: %: %.c
	$(CC) $(CFLAGS) -o $@ $^
