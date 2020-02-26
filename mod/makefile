
MDIR=build$/pudron$/mod
ifeq ($(PLATFORM),LINUX)
	DLL_MATH=libmath.so
	EXE_PD=./build/pudron/pd
else
	DLL_MATH=math.dll
	EXE_PD=build\pudron\pd.exe
endif
mod:error math
math:mod$/math.c include$/pdex.h
	$(CC) $(CFLAGS) -shared -fPIC mod$/math.c -o $(MDIR)$/$(DLL_MATH)
	$(EXE_PD) mod$/math.pd -m $(MDIR)$/math.pdm
error:mod$/error.pd
	$(EXE_PD) mod$/error.pd -m $(MDIR)$/error.pdm