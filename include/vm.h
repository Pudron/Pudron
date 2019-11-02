#ifndef _PD_VM_H_
#define _PD_VM_H_
#include"common.h"
typedef struct{
	char*name;
	Value val;
}Var;
LIST_DECLARE(Var)
typedef struct{
	Parser parser;
	ValueList stack;
	VarList var;
	intList enableClass;
	intList enableFunc;
}VM;
#endif