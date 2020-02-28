#ifndef _PD_CORE_H_
#define _PD_CORE_H_
#include"common.h"
#include"pdex.h"
#define MAX_STACK 1024
#ifdef LINUX
#include<dlfcn.h>
#include<unistd.h>
typedef void* Dllptr;
#define DLL_OPEN(f_name) dlopen(f_name,RTLD_LAZY)
#define DLL_GET(d_ptr,f_name) (DllFunc)dlsym(d_ptr,f_name)
#define DLL_CLOSE(d_ptr) dlclose(d_ptr)
#else
#include<windows.h>
typedef HINSTANCE Dllptr;
#define DLL_OPEN(f_name) LoadLibrary(TEXT(f_name))
#define DLL_GET(d_ptr,f_name) (DllFunc)GetProcAddress(d_ptr,TEXT(f_name))
#define DLL_CLOSE(d_ptr) FreeLibrary(d_ptr)
#endif
typedef void (*DllFunc)(_PDat*pdat);
LIST_DECLARE(DllFunc)
typedef struct{
    Dllptr dllptr;
    DllFuncList dflist;
}Dllinfo;
LIST_DECLARE(Dllinfo)
typedef Object* Arg;
LIST_DECLARE(Arg)
struct VMDef{
    int stackCount;
    Part part;
    int ptr;
    PartList plist;
    Object*stack[MAX_STACK];
    Object*this;
    PdSTD pstd;
    Unit*unit;
    DllinfoList dlist;
    char*path;
    intList jplist;
    Object*errObj;
};
typedef enum{
    ERR_INDEX,
    ERR_CALCULATION,
    ERR_FILE,
    ERR_ARGUMENT,
    ERR_MEMORY
}ErrorType;/*注意更改pdex.h中的enum,并更改error.pd*/
#define PUSH(objt) vm->stack[vm->stackCount++]=objt
#define POP() vm->stack[--vm->stackCount]
#define FUNC_DEF(name) void name(VM*vm,Unit*unit,Func*func){
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
#define METHOD_NAME_NOT_EQUAL "opNotEqual"
#define METHOD_NAME_GTHAN_EQUAL "opGthanEqual"/*由于不知道><==的返回值，只能设此*/
#define METHOD_NAME_LTHAN_EQUAL "opLthanEqual"
#define METHOD_NAME_REM "opRem"

void doexit(VM*vm,Unit*unit);
void vmError(VM*vm,wchar_t*text,...);
void reduceRef(VM*vm,Unit*unit,Object*obj);
Object*loadConst(VM*vm,Unit*unit,int index);
PdSTD makeSTD();
/*loadMember():当confirm为true时，未找到成员则报错，否则返回NULL*/
Object*loadMember(VM*vm,Object*this,char*name,bool confirm);
/*先从所属类查找,然后全局变量,最后局部变量,若查找到的对象未创建,则创建*/
Object*loadVar(VM*vm,Unit*unit,char*name);
Object*newObject(char type);
Object*newIntObject(int num);
Object*newDoubleObject(double numd);
Object*newClassObject(Class class);
Object*newFuncObject(Func func);
Object*newStringObject(VM*vm,wchar_t*wstr);
Object*newListObject(VM*vm,int count);
Object*newErrorObject(VM*vm,ErrorType id,wchar_t*msg,...);
void confirmObjectType(VM*vm,Object*obj,char type);
void setHash(VM*vm,HashList*hl,char*name,Object*obj);
void delObj(VM*vm,Unit*unit,Object*obj);
void callFunction(VM*vm,Unit*unit,Func func,int argc,...);
void freeHashList(VM*vm,Unit*unit,HashList*hl);
void makeSTDObject(VM*vm,PdSTD*pstd);
#endif