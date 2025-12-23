# Makefile for v7sh-amd64
# Compatible with both GNU Make and bmake (BSD)

CC = gcc
CFLAGS = -g -Wall -std=gnu89 -fcommon -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I. -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-implicit-function-declaration -Wno-implicit-int -Wno-return-type
LDFLAGS = 

SRCS = args.c blok.c builtin.c cmd.c ctype.c error.c expand.c \
       fault.c io.c macro.c main.c msg.c name.c print.c service.c \
       setbrk.c stak.c string.c test.c word.c xec.c
OBJS = $(SRCS:.c=.o)

PROG = v7sh

.SUFFIXES: .c .o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(PROG)
