
MDIR=build$/pudron$/mod
ifeq ($(PLATFORM),LINUX)
	DLL_MATH=libmath.so
	
else
	DLL_MATH=math.dll
	
endif
.PHONY:mod math error
mod:pd error math
math:mod$/math.c include$/pdex.h
	$(CC) $(CFLAGS) -shared -fPIC mod$/math.c -o $(MDIR)$/$(DLL_MATH)
	$(EXE_PD) mod$/math.pd -m $(MDIR)$/math.pdm
error:mod$/error.pd
	$(EXE_PD) mod$/error.pd -m $(MDIR)$/error.pdm