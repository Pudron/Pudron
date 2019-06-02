vpath %.h include
vpath %.c src
vpath %.o objs
CC=gcc
CFLAGS=-Iinclude -Wall -O2 -g
OBJS=main.o normal.o pio.o parse.o
pdc:$(OBJS)
	$(CC) $(addprefix objs/,$(OBJS)) -o pdc

$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o objs/$@

.PHONY:clean
clean:
#	-rm objs/*
	-del /Q objs