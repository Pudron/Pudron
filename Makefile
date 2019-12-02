vpath %.h include
vpath %.c src
vpath %.o objs
CC=gcc
CFLAGS=-Iinclude -Wall -O2 -g
OBJS=main.o common.o pio.o parser.o vm.o compiler.o
pd:$(OBJS)
	$(CC) $(addprefix objs/,$(OBJS)) -o pd

$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o objs/$@

.PHONY:lib
lib:lib/float.pd lib/meta.pd lib/string.pd lib/debug.pd
	./pd -l lib/meta.pd
	./pd -l lib/float.pd
	./pd -l lib/string.pd
	./pd -l lib/debug.pd

.PHONY:clean
clean:
	-rm objs/*
#	-del /Q objs