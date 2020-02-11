#ifndef _PD_VM_H_
#define _PD_VM_H_
#include"common.h"
#include"core.h"
void execute(VM*vm,Unit*unit);
VM newVM(char*fileName,char*path,PdSTD pstd);
void popStack(VM*vm,Unit*unit,int num);
#endif