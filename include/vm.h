#ifndef _PD_VM_H_
#define _PD_VM_H_
#include"common.h"
#include"pio.h"
typedef struct{
	char*name;
	Value val;
}Var;
LIST_DECLARE(Var)
typedef struct{
	int funcIndex;
	int varIndex;
	int stackIndex;
}Field;
LIST_DECLARE(Field)
typedef struct{
	Value*var;
    int varCount;
    int varBasis;
	int refCount;
	bool isUsed;
	char*str;
}Ref;
LIST_DECLARE(Ref)
typedef struct{
	PartList partList;
    intList clist;
    SymbolList symList;
    FuncList funcList;
    ClassList classList;
	ModuleList moduleList;
	
	ValueList stack;
	VarList varList;
	RefList refList;
	intList enableFunc;
	FieldList fields;
	intList loopList;
	Module curModule;
	ModuleList mlist;
	intList funcPartList;
	int ptr,curPart;
	int curVar;
	intList vlist;
}VM;
void initVM(VM*vm,Parser parser);
void exitVM(VM*vm);
Value makeValue(VM*vm,int class);
void exeFunc(VM*vm,Func func,int argCount,bool isMethod,bool isInit,int curPart);
void reportVMError(VM*vm,char*text,int curPart);
void execute(VM*vm,intList clist);
#endif