#ifndef _PD_CORE_H_
#define _PD_CORE_H_
#include"common.h"
#define MAX_STACK 1024

struct VMDef{
    int stackCount;
    intList loopList;
    Part part;
    int ptr;
    PartList plist;
    Object*stack[MAX_STACK];
    Object*this;
};
/*注意：argv不包括this和argv*/
#define PUSH(objt) vm->stack[vm->stackCount++]=objt
#define POP() vm->stack[--vm->stackCount]
#define FUNC_DEF(name) void name(VM*vm,Unit*unit){
#define FUNC_END() PUSH(newIntObject(0));return;}

#define METHOD_NAME_INIT "opInit"
#define METHOD_NAME_DESTROY "opDestroy"
#define METHOD_NAME_SUBSCRIPT "opSubscript"
#define METHOD_NAME_ADD "opAdd"
#define METHOD_NAME_SUB "opSub"
#define METHOD_NAME_MUL "opMul"
#define METHOD_NAME_DIV "opDiv"
#define METHOD_NAME_AND "opAnd"
#define METHOD_NAME_OR "opOr"
#define METHOD_NAME_CAND "opCand"
#define METHOD_NAME_COR "opCor"
#define METHOD_NAME_LEFT "opLeft"
#define METHOD_NAME_RIGHT "opRight"
#define METHOD_NAME_EQUAL "opEqual"
#define METHOD_NAME_GTHAN "opGthan"
#define METHOD_NAME_LTHAN "opLthan"
#define METHOD_NAME_REM "opRem"

void vmError(VM*vm,char*text,...);
void reduceRef(VM*vm,Unit*unit,Object*obj);
Object*loadConst(VM*vm,Unit*unit,int index);
PdSTD makeSTD();
/*loadMember():当confirm为true时，未找到成员则报错，否则返回NULL*/
Object*loadMember(VM*vm,Object*this,char*name,bool confirm);
Object*loadVar(VM*vm,Unit*unit,char*name);
Object*newObject(char type);
Object*newIntObject(int num);
Object*newDoubleObject(double numd);
Object*newClassObject(Class class);
Object*newFuncObject(Func func);
#endif