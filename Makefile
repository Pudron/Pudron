vpath %.h include
vpath %.c src
vpath %.o build/objs
PLATFORM ?= WINDOWS
CC=gcc
CFLAGS=-Iinclude -Wall -O2 -g -D$(PLATFORM) -DDEBUG
LIBS=
OBJS=main.o common.o pio.o parser.o compiler.o core.o vm.o
ifeq ($(PLATFORM),LINUX)
	EXE=./
	/ =$(strip /)
	LIBS+=-ldl
	PD_FILE=pd
	RM=rm -r
else
	EXE=
	/ =$(strip \)
	PD_FILE=pd.exe
	RM=del /Q
endif
ifneq (build$/objs,$(wildcard build$/objs))
	MKD1=mkdir build$/objs
else
	MKD1=
endif
ifneq (build$/pudron,$(wildcard build$/pudron))
	MKD2=mkdir build$/pudron
	MKD3=mkdir build$/pudron$/lib
else
	MKD2=
	MKD3=
endif
all:mkd pd lib

.PHONY:mkd
mkd:
	$(MKD1)
	$(MKD2)
	$(MKD3)

pd:$(OBJS)
	$(CC) $(addprefix build$/objs$/,$(OBJS)) -o build$/pudron$/pd $(LIBS)

$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o build$/objs$/$@

.PHONY:test
test:test$/test.pd
	$(EXE)build$/pudron$/pd test$/test.pd

.PHONY:debug
debug:build$/pudron$/$(PD_FILE) test$/test.pd
	gdb --args build$/pudron$/pd test$/test.pd

.PHONY:clean
clean:
	-$(RM) build$/objs
	-$(RM) build$/pudron
