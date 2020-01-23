#ifndef _PD_CORE_H_
#define _PD_CORE_H_
#include"common.h"
#define MAX_STACK 1024
#define STD_CLASS_COUNT 6
#define CLASS_INT 0
#define CLASS_CLASS 1
#define CLASS_FUNCTION 2
#define CLASS_DOUBLE 3
#define CLASS_STRING 4
#define CLASS_LIST 5
typedef struct{
    int hashName;
    Object*obj;
}Stack;
struct VMDef{
    Class stdclass[STD_CLASS_COUNT];
    int stackCount;
    Stack stack[MAX_STACK];
    Part part;
    int ptr;
};
#define PUSH(objt) vm->stack[vm->stackCount].hashName=hashString("_pd_stack_");vm->stack[vm->stackCount++].obj=objt
#define POP() vm->stack[--vm->stackCount].obj
#define FUNC_DEF(name) void name(VM*vm,Unit*unit)
#define PD_ERROR(text) vmError(vm,text)
#define PD_RETURN(obj) PUSH(obj);return
void vmError(VM*vm,char*text);
bool compareClassStd(VM*vm,Object*obj,int class);
Object*newObjectStd(VM*vm,int class);
void reduceRef(Object*obj);
#endif