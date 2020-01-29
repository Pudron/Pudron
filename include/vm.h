#ifndef _PD_VM_H_
#define _PD_VM_H_
#include"common.h"
#include"core.h"
void callFunction(VM*vm,Func func,int argc);
void execute(VM*vm,Unit*unit);
VM newVM(char*fileName);
void popStack(VM*vm,int num);
#endif