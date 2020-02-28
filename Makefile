vpath %.h include
vpath %.c src
vpath %.o build/objs
BUILD ?= DEBUG
PLATFORM ?= WINDOWS
CC=gcc
CFLAGS=-Iinclude -Wall -D$(PLATFORM) -D$(BUILD) -std=c18
LIBS=-liconv
OBJS=main.o common.o pio.o parser.o compiler.o core.o vm.o
DIR_SRC=src
DIR_OBJS=build/objs
OBJS_TXT=build/objs/objs.txt
ifeq ($(PLATFORM),LINUX)
	LIBS+=-ldl
	RM=rm -r
	EXE_PD=./build/pudron/pd
else
	RM=rmdir /s /q
	EXE_PD=build\pudron\pd.exe
endif

ifeq ($(BUILD),DEBUG)
	CFLAGS+= -g
else
	CFLAGS+= -O2
endif

.PHONY:all test debug clean
all:$(EXE_PD) mod

$(EXE_PD):$(OBJS) build/pudron
	$(CC) $(addprefix $(DIR_OBJS)/,$(OBJS)) $(LIBS) -o $(EXE_PD)

build:
ifneq (build,$(wildcard build))
	mkdir build
endif
$(OBJS_TXT):build
ifneq (build/objs,$(wildcard build/objs))
	mkdir build/objs
	@echo "this objs directory" > $(OBJS_TXT)
endif
build/pudron:build
ifneq (build/pudron,$(wildcard build/pudron))
	mkdir build/pudron
endif
main.o:main.c $(OBJS_TXT) common.h compiler.h core.h vm.h
	$(CC) -c $(CFLAGS) $< -o $(DIR_OBJS)/$@
common.o:common.c $(OBJS_TXT) common.h
	$(CC) -c $(CFLAGS) $< -o $(DIR_OBJS)/$@
pio.o:pio.c $(OBJS_TXT) common.h pio.h
	$(CC) -c $(CFLAGS) $< -o $(DIR_OBJS)/$@
parser.o:parser.c $(OBJS_TXT) common.h compiler.h parser.h
	$(CC) -c $(CFLAGS) $< -o $(DIR_OBJS)/$@
compiler.o:compiler.c $(OBJS_TXT) common.h compiler.h parser.h core.h pio.h
	$(CC) -c $(CFLAGS) $< -o $(DIR_OBJS)/$@
core.o:core.c $(OBJS_TXT) common.h pio.h pdex.h core.h vm.h
	$(CC) -c $(CFLAGS) $< -o $(DIR_OBJS)/$@
vm.o:vm.c $(OBJS_TXT) common.h core.h vm.h
	$(CC) -c $(CFLAGS) $< -o $(DIR_OBJS)/$@

include mod/mod.mk

test:
	$(EXE_PD) test/test.pd

debug:
	gdb --args $(EXE_PD) test/test.pd

clean:
	-rm -r build