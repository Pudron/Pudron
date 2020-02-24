
MDIR=build$/pudron$/mod
ifeq ($(PLATFORM),LINUX)
	DLL_FILE=libfile.so
	EXE_PD=./build/pudron/pd
else
	DLL_FILE=file.dll
	EXE_PD=build\pudron\pd.exe
endif
mod:file error
file:mod$/file.c include$/pdex.h
	$(CC) $(CFLAGS) -shared -fPIC mod$/file.c -o $(MDIR)$/$(DLL_FILE)
	$(EXE_PD) mod$/file.pd -m $(MDIR)$/file.pdm
error:mod$/error.pd
	$(EXE_PD) mod$/error.pd -m $(MDIR)$/error.pdm