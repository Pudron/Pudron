
MDIR=build$/pudron$/mod
ifeq ($(PLATFORM),LINUX)
	DLL_FILE=libfile.so
	EXE_PD=./build/pudron/pd
else
	DLL_FILE=file.dll
	EXE_PD=build\pudron\pd.exe
endif
mod:file
file:mod$/file.c include$/pdex.h
	$(CC) $(CFLAGS) -shared mod$/file.c -D$(PLATFORM) -o $(MDIR)$/$(DLL_FILE)
	$(EXE_PD) mod$/file.pd -m $(MDIR)$/file.pdm