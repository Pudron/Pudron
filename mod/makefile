DIR_MOD=build/pudron/mod
MOD_TXT=$(DIR_MOD)/mod.txt
ifeq ($(PLATFORM),LINUX)
	DLL_MATH=libmath.so
else
	DLL_MATH=math.dll
endif
.PHONY:mod
mod:$(MOD_TXT) $(DIR_MOD)/error.pdm $(DIR_MOD)/math.pdm
$(MOD_TXT):build/pudron
ifneq ($(DIR_MOD),$(wildcard $(DIR_MOD)))
	mkdir $(DIR_MOD)
	@echo "this is mod file" > $(MOD_TXT)
endif
$(DIR_MOD)/math.pdm:mod/math.c include/pdex.h mod/math.pd $(MOD_TXT) $(EXE_PD)
	$(CC) $(CFLAGS) -shared -fPIC mod/math.c -o $(DIR_MOD)/$(DLL_MATH)
	$(EXE_PD) mod/math.pd -m $(DIR_MOD)/math.pdm
$(DIR_MOD)/error.pdm:mod/error.pd $(MOD_TXT) $(EXE_PD)
	$(EXE_PD) mod/error.pd -m $(DIR_MOD)/error.pdm