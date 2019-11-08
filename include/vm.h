#ifndef _PD_VM_H_
#define _PD_VM_H_
#include"common.h"
typedef struct{
	char*name;
	Value val;
}Var;
LIST_DECLARE(Var)
typedef struct{
	int FuncIndex;
	int varIndex;
}Field;
LIST_DECLARE(Field)
typedef struct{
	Parser parser;
	ValueList stack;
	VarList var;
	intList enableFunc;
	FieldList fields;
	intList retList;/*记录当前field,返回时逐个释放到此*/
}VM;
void initVM(VM*vm,Parser parser);
void reportVMError(VM*vm,char*text,int curPart);
#endif