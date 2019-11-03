vpath %.h include
vpath %.c src
vpath %.o objs
CC=gcc
CFLAGS=-Iinclude -Wall -O2 -g
OBJS=main.o common.o pio.o parser.o
pd:$(OBJS)
	$(CC) $(addprefix objs/,$(OBJS)) -o pd

$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o objs/$@

.PHONY:clean
clean:
#	-rm objs/*
	-del /Q objs