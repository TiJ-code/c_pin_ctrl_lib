CC=gcc
CFLAGS=-Wall -Wextra -pedantic
LDFLAGS=-lpigpiod_if2 -lpthread

.PHONE: all clean

all: build

build: pin_ctrl.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f pin_ctrl
