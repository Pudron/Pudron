#include"vm.h"
#ifdef DEBUG
#define ASSERT(condition,message,...) \
    if(condition){\
        printf(message,##__VA_ARGS__);\
        getchar();\
    }
#else
#define ASSERT(condition,message,...)
#endif
#define checkStack(num) ASSERT(vm->stackCount<num,"%d:expected %d slots but only %d slots in stack.",vm->ptr,num,vm->stackCount)
VM newVM(){
    VM vm;
    vm.stackCount=0;
    return vm;
}
void callFunction(VM*vm,Func func,int argc){
    Unit unit=getFuncUnit(func);

}
#define EXE_OPT(opt) \
    if(compareClassStd(vm,a,CLASS_INT)){\
        if(compareClassStd(vm,b,CLASS_INT)){\
            c=newObjectStd(vm,CLASS_INT);\
            c->num=a->num opt b->num;\
        }else if(compareClassStd(vm,b,CLASS_DOUBLE)){\
            c=newObjectStd(vm,CLASS_DOUBLE);\
            c->numd=a->num opt b->numd;\
        }else{\
            vmError(vm,"unsupported operation for int.");\
        }\
        PUSH(c);\
    }else if(compareClassStd(vm,a,CLASS_DOUBLE)){\
        c=newObjectStd(vm,CLASS_DOUBLE);\
        if(compareClassStd(vm,b,CLASS_INT)){\
            c->numd=a->numd opt b->num;\
        }else if(compareClassStd(vm,b,CLASS_DOUBLE)){\
            c->numd=a->numd opt b->numd;\
        }else{\
            vmError(vm,"unsupported operation for double.");\
        }\
        PUSH(c);\
    }else{\
        isFunc=true;\
    }
#define EXE_OPT_BIT(opt) \
    if(compareClassStd(vm,a,CLASS_INT)){\
        if(compareClassStd(vm,b,CLASS_INT)){\
            c=newObjectStd(vm,CLASS_INT);\
            c->num=a->num opt b->num;\
        }else{\
            vmError(vm,"unsupported operation for int.");\
        }\
        PUSH(c);\
    }else{\
        isFunc=true;\
    }
void exeOpt(VM*vm,int opcode){
    checkStack(2);
    Object*a,*b,*c=NULL;
    b=POP();
    a=POP();
    bool isFunc=false;
    switch(opcode){
        case OPCODE_ADD:
            EXE_OPT(+)
            break;
        case OPCODE_SUB:
            EXE_OPT(-)
            break;
        case OPCODE_MUL:
            EXE_OPT(*)
            break;
        case OPCODE_DIV:
            EXE_OPT(/)
            break;
        case OPCODE_AND:
            EXE_OPT_BIT(&)
            break;
        case OPCODE_OR:
            EXE_OPT_BIT(|)
            break;
        case OPCODE_CAND:
            EXE_OPT(&&)
            break;
        case OPCODE_COR:
            EXE_OPT(||)
            break;
        case OPCODE_LEFT:
            EXE_OPT_BIT(<<)
            break;
        case OPCODE_RIGHT:
            EXE_OPT_BIT(>>)
            break;
        case OPCODE_EQUAL:
            EXE_OPT(==)
            break;
        case OPCODE_GTHAN:
            EXE_OPT(>)
            break;
        case OPCODE_LTHAN:
            EXE_OPT(<)
            break;
        case OPCODE_NOT_EQUAL:
            EXE_OPT(!=)
            break;
        case OPCODE_GTHAN_EQUAL:
            EXE_OPT(>=)
            break;
        case OPCODE_LTHAN_EQUAL:
            EXE_OPT(<=)
            break;
        case OPCODE_REM:
            EXE_OPT_BIT(%)
            break;
        default:
            break;
    }
    if(isFunc){
        PUSH(a);
        PUSH(b);
        int ind=a->class->optID[opcode-1];
        if(ind<0){
            vmError(vm,"undefined operation.");
        }
        c=a->objs[ind];
        if(!compareClassStd(vm,c,CLASS_FUNCTION)){
            vmError(vm,"expected operation method.");
        }
        callFunction(vm,c->func,2);
    }else{
        reduceRef(b);
        reduceRef(a);
    }
}
void execute(VM*vm,Unit*unit){
    char temp[50];
    int c;
    for(int i=0;i<unit->clist.count;i++){
        vm->part=unit->plist.vals[i];
        c=unit->clist.vals[++i];
        vm->ptr=i;
        switch(c){
            case OPCODE_NOP:
                break;
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_CAND:
            case OPCODE_COR:
            case OPCODE_LEFT:
            case OPCODE_RIGHT:
            case OPCODE_EQUAL:
            case OPCODE_GTHAN:
            case OPCODE_LTHAN:
            case OPCODE_NOT_EQUAL:
            case OPCODE_GTHAN_EQUAL:
            case OPCODE_LTHAN_EQUAL:
            case OPCODE_REM:
                exeOpt(vm,c);
                break;
            default:
                sprintf(temp,"unknown opcode %d.",c);
                vmError(vm,temp);
                break;
        }
    }
}