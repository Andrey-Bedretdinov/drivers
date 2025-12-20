CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra

all: can_send can_recv

can_send: can_send.c
	$(CC) $(CFLAGS) -o $@ $<

can_recv: can_recv.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f can_send can_recv