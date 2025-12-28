# Makefile for v7sh-amd64 (Linux Hybrid Port)
# Supports GCC for stability and PCC for historical auditing.

CC      ?= gcc
CFLAGS  += -g -O0 -D_GNU_SOURCE -I. -I./include -fcommon -fno-stack-protector -fno-builtin-malloc
LDFLAGS += -g

SRCS = args.c blok.c builtin.c cmd.c ctype.c error.c expand.c \
       fault.c io.c macro.c main.c msg.c name.c print.c service.c \
       setbrk.c stak.c string.c test.c word.c xec.c
OBJS = $(SRCS:.c=.o)

PROG = v7sh

.PHONY: all clean vintage paranoid

# Default: GCC build
all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

# Vintage: PCC build
vintage: clean
	$(MAKE) CC=pcc CFLAGS="-g -O0 -I. -I./include"

# Verification: Run the Super Paranoid suite
paranoid: clean $(PROG)
	./super_paranoid_v7.sh

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(PROG)
