vpath %.h include
vpath %.c src
vpath %.o build/objs
BUILD ?= RELEASE
PLATFORM ?= WINDOWS
CC=gcc
CFLAGS=-Iinclude -Wall -D$(PLATFORM) -D$(BUILD)
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
ifeq ($(BUILD),RELEASE)
	CFLAGS+= -O2
else
	CFLAGS+= -g
endif
ifneq (build$/objs,$(wildcard build$/objs))
	MKD1=mkdir build$/objs
else
	MKD1=
endif
ifneq (build$/pudron,$(wildcard build$/pudron))
	MKD2=mkdir build$/pudron
	MKD3=mkdir build$/pudron$/mod
else
	MKD2=
	MKD3=
endif
all:mkd pd mod

.PHONY:mkd
mkd:
	$(MKD1)
	$(MKD2)
	$(MKD3)

pd:$(OBJS)
	$(CC) $(addprefix build$/objs$/,$(OBJS)) -o build$/pudron$/pd $(LIBS)

$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o build$/objs$/$@

include mod/makefile

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
