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
lib:lib/src/float.pd lib/src/meta.pd lib/src/string.pd lib/src/debug.pd
	./pd -l lib/src/meta.pd -o lib/meta.pdl
	./pd -l lib/src/float.pd -o lib/float.pdl
	./pd -l lib/src/string.pd -o lib/string.pdl
	./pd -l lib/src/debug.pd -o lib/debug.pdl
	./pd -l lib/src/list.pd -o lib/list.pdl
	./pd -l lib/src/file.pd -o lib/file.pdl

.PHONY:clean
clean:
	-rm objs/*
#	-del /Q objs