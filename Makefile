CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -pedantic
TARGET  = mini-shell
SRCS    = main.c parser.c executor.c builtins.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c shell.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
