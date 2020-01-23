#include"core.h"
void vmError(VM*vm,char*text){
    Msg msg;
    Part part=vm->part;
    msg.fileName=part.fileName;
    msg.code=part.code;
    msg.line=part.line;
    msg.column=part.column;
    msg.start=part.start;
    msg.end=part.end;
    strcpy(msg.text,text);
    reportMsg(msg);
}
Func newFunc(){
    Func func;
    func.exe=NULL;
    LIST_INIT(func.argList)
    Unit unit=newUnit(0);
    setFuncUnit(&func,unit);
    return func;
}
Class newClass(char*name){
    Class class;
    class.name=name;
    class.hashName=hashString(name);
    LIST_INIT(class.varList)
    memset(class.optID,-1,OPT_METHOD_COUNT+1);
    class.initID=-1;
    class.destroyID=-1;
    class.initFunc=newFunc();
    return class;
}
bool compareClassStd(VM*vm,Object*obj,int class){
    if(obj->class->hashName==vm->stdclass[class].hashName){
        return true;
    }
    return false;
}
Object*newObjectStd(VM*vm,int class){
    Class*classd=&vm->stdclass[class];
    Object*obj=(Object*)malloc(sizeof(Object));
    obj->class=classd;
    obj->varCount=classd->varList.count;
    obj->objs=(obj->varCount>0)?((Object**)malloc(sizeof(Object*)*obj->varCount)):NULL;
    obj->refCount=1;
    return obj;
}
void reduceRef(Object*obj){
    obj->refCount--;
    if(obj->refCount<=0){
        for(int i=0;i<obj->varCount;i++){
            reduceRef(obj->objs[i]);
        }
        if(obj->varCount>0){
            free(obj->objs);
        }
        free(obj);
    }
}
Object*loadConst(VM*vm,Unit*unit,int index){
    Object*obj=NULL;
    char temp[50];
    Const con=unit->constList.vals[index];
    switch(con.type){
        case CONST_INT:
            obj=newObjectStd(vm,CLASS_INT);
            obj->num=con.num;
            break;
        case CONST_DOUBLE:
            obj=newObjectStd(vm,CLASS_DOUBLE);
            obj->numd=con.numd;
            break;
        case CONST_STRING:
            obj=newObjectStd(vm,CLASS_STRING);
            obj->str=(char*)malloc(strlen(con.str)+1);
            strcpy(obj->str,con.str);
            break;
        case CONST_FUNCTION:
            obj=newObjectStd(vm,CLASS_FUNCTION);
            obj->func=con.func;
            break;
        case CONST_CLASS:
            obj=newObjectStd(vm,CLASS_CLASS);
            obj->classd=&con.classd;
            break;
        default:
            sprintf(temp,"unknown constant type:%d.",con.type);
            vmError(vm,temp);
            break;
    }
    return obj;
}
FUNC_DEF(std_init){
    Object*obj;
    for(int i=0;i<unit->constList.count;i++){
        obj=loadConst(vm,unit,i);
        PUSH(obj);
    }
}
void addClassFunc(Class*class,char*name,void*exe,int optID){
    Func func=newFunc();
    func.exe=exe;
    Var var={false,name,hashString(name)};
    LIST_ADD(class->varList,Var,var)
    if(optID>=1){
        class->optID[optID-1]=class->varList.count-1;
    }
    Const con;
    con.type=CONST_FUNCTION;
    con.func=func;
    LIST_ADD(class->initFunc.constList,Const,con)
}
/*#define INT_FUNC_BIT_DEF(name,opt) \
    FUNC_DEF(name){\
        Object*obj=POP();\
        Object*this=POP();\
        Object*result=NULL;\
        if(compareClassStd(vm,obj,CLASS_INT)){\
            result=newObjectStd(vm,CLASS_INT);\
            result->num=this->num opt obj->num;\
        }else{\
            PD_ERROR("unsupported operation for int.");\
        }\
        reduceRef(obj);\
        reduceRef(this);\
        PD_RETURN(result);\
    }
#define INT_FUNC_DEF(name,opt) \
    FUNC_DEF(name){\
        Object*obj=POP();\
        Object*this=POP();\
        Object*result=NULL;\
        if(compareClassStd(vm,obj,CLASS_INT)){\
            result=newObjectStd(vm,CLASS_INT);\
            result->num=this->num opt obj->num;\
        }else if(compareClassStd(vm,obj,CLASS_DOUBLE)){\
            result=newObjectStd(vm,CLASS_DOUBLE);\
            result->numd=(double)this->num opt obj->numd;\
        }else{\
            PD_ERROR("unsupported operation for int.");\
        }\
        reduceRef(obj);\
        reduceRef(this);\
        PD_RETURN(result);\
    }
INT_FUNC_DEF(INT_ADD,+)
INT_FUNC_DEF(INT_SUB,-)
INT_FUNC_DEF(INT_MUL,*)
INT_FUNC_DEF(INT_DIV,/)
INT_FUNC_BIT_DEF(INT_AND,&)
INT_FUNC_BIT_DEF(INT_OR,|)
INT_FUNC_DEF(INT_CAND,&&)
INT_FUNC_DEF(INT_COR,||)
INT_FUNC_BIT_DEF(INT_LEFT,<<)
INT_FUNC_BIT_DEF(INT_RIGHT,>>)
INT_FUNC_DEF(INT_EQUAL,==)
INT_FUNC_DEF(INT_GTHAN,>)
INT_FUNC_DEF(INT_LTHAN,<)
INT_FUNC_DEF(INT_NOT_EQUAL,!=)
INT_FUNC_DEF(INT_GTHAN_EQUAL,>=)
INT_FUNC_DEF(INT_LTHAN_EQUAL,<=)
INT_FUNC_BIT_DEF(INT_REM,%)
#define DOUBLE_FUNC_DEF(name,opt) \
    FUNC_DEF(name){\
        Object*obj=POP();\
        Object*this=POP();\
        Object*result=NULL;\
        if(compareClassStd(vm,obj,CLASS_INT)){\
            result=newObjectStd(vm,CLASS_DOUBLE);\
            result->num=this->numd opt obj->num;\
        }else if(compareClassStd(vm,obj,CLASS_DOUBLE)){\
            result=newObjectStd(vm,CLASS_DOUBLE);\
            result->num=this->numd opt obj->numd;\
        }else{\
            PD_ERROR("unsupported operation for double.");\
        }\
        reduceRef(obj);\
        reduceRef(this);\
        PD_RETURN(result);\
    }
DOUBLE_FUNC_DEF(DOUBLE_ADD,+)
DOUBLE_FUNC_DEF(DOUBLE_SUB,-)
DOUBLE_FUNC_DEF(DOUBLE_MUL,*)
DOUBLE_FUNC_DEF(DOUBLE_DIV,/)
DOUBLE_FUNC_DEF(DOUBLE_CAND,&&)
DOUBLE_FUNC_DEF(DOUBLE_COR,||)
DOUBLE_FUNC_DEF(DOUBLE_EQUAL,==)
DOUBLE_FUNC_DEF(DOUBLE_GTHAN,>)
DOUBLE_FUNC_DEF(DOUBLE_LTHAN,<)
DOUBLE_FUNC_DEF(DOUBLE_NOT_EQUAL,!=)
DOUBLE_FUNC_DEF(DOUBLE_GTHAN_EQUAL,>=)
DOUBLE_FUNC_DEF(DOUBLE_LTHAN_EQUAL,<=)
#define ADD_INT_FUNC(name) addClassFunc(&class,#name,INT_##name,OPCODE_##name)
#define ADD_DOUBLE_FUNC(name) addClassFunc(&class,#name,DOUBLE_##name,OPCODE_##name)*/
void makeSTD(VM*vm){
    Class class;
    /*int definition*/
    class=newClass("int");
    class.initFunc=newFunc();
    class.initFunc.exe=NULL;
    vm->stdclass[CLASS_INT]=class;
    /*double definition*/
    class=newClass("double");
    class.initFunc=newFunc();
    class.initFunc.exe=NULL;
    vm->stdclass[CLASS_DOUBLE]=class;
}