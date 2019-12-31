vpath %.h include
vpath %.c src
vpath %.o build/objs
CC=gcc
CFLAGS=-Iinclude -Wall -O2 -g -DLINUX
LIBS=-ldl
OBJS=main.o common.o pio.o parser.o vm.o compiler.o
all:mkd pd lib

.PHONY:mkd
mkd:
	-mkdir build/objs
	-mkdir build/pudron
	-mkdir build/pudron/lib

pd:$(OBJS)
	$(CC) $(addprefix build/objs/,$(OBJS)) -o build/pudron/pd $(LIBS)

$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o build/objs/$@

.PHONY:lib
lib:lib/float.pd lib/meta.pd lib/string.pd lib/debug.pd
	./build/pudron/pd -l lib/meta.pd -o build/pudron/lib/meta.pdl
	./build/pudron/pd -l lib/float.pd -o build/pudron/lib/float.pdl
	./build/pudron/pd -l lib/string.pd -o build/pudron/lib/string.pdl
	./build/pudron/pd -l lib/debug.pd -o build/pudron/lib/debug.pdl
	./build/pudron/pd -l lib/list.pd -o build/pudron/lib/list.pdl
	./build/pudron/pd -l lib/file.pd -o build/pudron/lib/file.pdl
	./build/pudron/pd -l lib/dll.pd -o build/pudron/lib/dll.pdl
.PHNOY:test
test:test/test.pd
	./build/pudron/pd test/test.pd

.PHONY:clean
clean:
	-rm -r build/objs
	-rm -r build/pudron
#	-del /Q objs