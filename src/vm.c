#include"vm.h"
void initVM(VM*vm,Parser parser){
    vm->parser=parser;
    LIST_INIT(vm->stack,Value)
    LIST_INIT(vm->memList,Value)
    LIST_INIT(vm->vars,Var)
    LIST_INIT(vm->enableFunc,int)
    LIST_INIT(vm->fields,Field)
    LIST_INIT(vm->retList,int)
}
void reportVMError(VM*vm,char*text,int curPart){
    Msg msg;
    Part part=vm->parser.partList.vals[curPart];
    msg.start=part.start;
    msg.end=part.end;
    msg.line=part.line;
    msg.column=part.column;
    strcpy(msg.text,text);
    reportMsg(&vm->parser,msg);
}
inline void loadConst(VM*vm,int arg,int curPart){
    Symbol symbol=vm->parser.symList.vals[arg];
    Value value;
    switch (symbol.type)
    {
    case SYM_INT:
        value.class=CLASS_INT;
        value.num=symbol.num;
        break;
    case SYM_FLOAT:
        value.class=CLASS_FLOAT;
        value.numf=symbol.numf;
        break;
    case SYM_STRING:
        value.class=CLASS_STRING;
        value.str=symbol.str;
        break;
    default:
        reportVMError(vm,"LOAD_CONST:unknown symbol type.",curPart);
        break;
    }
    value.var=NULL;
    LIST_ADD(vm->stack,Value,value)
}
inline void loadVal(VM*vm,int arg,int curPart){
    Symbol symbol=vm->parser.symList.vals[arg];
    char temp[50];
    if(symbol.type!=SYM_STRING){
        reportVMError(vm,"LOAD_VAL:symbol type must be string.",curPart);
    }
    bool isFound=false;
    for(int i=vm->vars.count-1;i>=0;i--){
        if(strcmp(symbol.str,vm->vars.vals[i].name)==0){
            if(vm->vars.vals[i].val.var!=NULL){
                vm->memList.vals[vm->vars.vals[i].val.refID].refCount++;
            }
            LIST_ADD(vm->stack,Value,vm->vars.vals[i].val)
            isFound=true;
            break;
        }
    }
    if(!isFound){
        sprintf(temp,"variable \"%s\" not found.",symbol.str);
        reportVMError(vm,temp,curPart);
    }
}
inline void loadAttr(VM*vm,int arg,int curPart){
    Symbol symbol=vm->parser.symList.vals[arg];
    char temp[50];
    if(symbol.type!=SYM_STRING){
        reportVMError(vm,"LOAD_VAL:symbol type must be string.",curPart);
    }
    Value a=vm->stack.vals[vm->stack.count-1];
    bool isFound=false;
    for(int i=0;i<vm->parser.classList.vals[a.class].var.count;i++){
        if(strcmp(symbol.str,vm->parser.classList.vals[a.class].var.vals[i])==0){
            isFound=true;
            vm->memList.vals[a.refID].refCount--;
            if(a.var[i].var!=NULL){
                vm->memList.vals[a.var[i].refID].refCount++;
            }
            vm->stack.vals[vm->stack.count-1]=a.var[i];
            break;
        }
    }
    if(!isFound){
        sprintf(temp,"member \"%s\" not found.",symbol.str);
        reportVMError(vm,temp,curPart);
    }
}
#define EXE_OPT(opt,ind) \
    a=vm->stack.vals[vm->stack.count-2];\
    b=vm->stack.vals[vm->stack.count-1];\
    switch(a.class){\
        case CLASS_INT:\
            switch(b.class){\
                case CLASS_INT:\
                    a.num=a.num opt b.num;\
                    break;\
                case CLASS_FLOAT:\
                    a.class=CLASS_FLOAT;\
                    a.numf=a.num opt b.numf;\
                    break;\
                default:\
                    reportVMError(vm,"unsupport int operation.",curPart);\
            }\
            break;\
        case CLASS_FLOAT:\
            switch(b.class){\
                case CLASS_INT:\
                    a.numf=a.numf opt b.num;\
                    break;\
                case CLASS_FLOAT:\
                    a.numf=a.numf opt b.numf;\
                    break;\
                default:\
                    reportVMError(vm,"unsupport float operation.",curPart);\
            }\
            break;\
        default:\
            if(vm->parser.classList.vals[a.class].optMethod[ind].clist.count<=0){\
                sprintf(temp,"unsupport %s operation.",vm->parser.classList.vals[a.class].name);\
                reportVMError(vm,temp,curPart);\
            }\
            LIST_ADD(vm->retList,int,vm->fields.count)\
            execute(vm,vm->parser.classList.vals[a.class].optMethod[ind].clist);\
    }\
    LIST_SUB(vm->stack,Value)\
    vm->stack.vals[vm->stack.count-1]=a;

#define EXE_OPT_BIT(opt,ind) \
    a=vm->stack.vals[vm->stack.count-2];\
    b=vm->stack.vals[vm->stack.count-1];\
    if(a.class==CLASS_INT){\
        if(b.class!=CLASS_INT){\
            reportVMError(vm,"expected an integer to operate.",curPart);\
        }\
        a.num= a.num opt b.num;\
    }else{\
        if(vm->parser.classList.vals[a.class].optMethod[ind].clist.count<=0){\
            sprintf(temp,"unsupport %s operation.",vm->parser.classList.vals[a.class].name);\
            reportVMError(vm,temp,curPart);\
        }\
        LIST_ADD(vm->retList,int,vm->fields.count)\
        execute(vm,vm->parser.classList.vals[a.class].optMethod[ind].clist);\
    }

void execute(VM*vm,intList clist){
    int opcode,curPart;
    char temp[50];
    Value a,b;
    for(int i=0;i<clist.count;i++){
        curPart=clist.vals[i++];
        opcode=clist.vals[i];
        switch(opcode){
            case OPCODE_NOP:
                break;
            case OPCODE_ADD:
                EXE_OPT(+,OPCODE_ADD)
                break;
            case OPCODE_SUB:
                EXE_OPT(-,OPCODE_SUB)
                break;
            case OPCODE_MUL:
                EXE_OPT(*,OPCODE_MUL)
                break;
            case OPCODE_DIV:
                EXE_OPT(/,OPCODE_DIV)
                break;
            case OPCODE_AND:
                EXE_OPT_BIT(&,OPCODE_AND)
                break;
            case OPCODE_OR:
                EXE_OPT_BIT(|,OPCODE_OR)
                break;
            case OPCODE_LEFT:
                EXE_OPT_BIT(<<,OPCODE_LEFT)
                break;
            case OPCODE_RIGHT:
                EXE_OPT_BIT(>>,OPCODE_RIGHT)
                break;
            case OPCODE_EQUAL:
                EXE_OPT(==,OPCODE_EQUAL)
                break;
            case OPCODE_GTHAN:
                EXE_OPT(>,OPCODE_GTHAN)
                break;
            case OPCODE_LTHAN:
                EXE_OPT(<,OPCODE_LTHAN)
                break;
            case OPCODE_NOT_EQUAL:
                EXE_OPT(!=,OPCODE_NOT_EQUAL)
                break;
            case OPCODE_GTHAN_EQUAL:
                EXE_OPT(>=,OPCODE_GTHAN_EQUAL)
                break;
            case OPCODE_LTHAN_EQUAL:
                EXE_OPT(<=,OPCODE_LTHAN_EQUAL)
                break;
            case OPCODE_REM:
                EXE_OPT_BIT(%,OPCODE_REM)
                break;
            case OPCODE_SUBS:
                a=vm->stack.vals[vm->stack.count-1];
                if(a.class==CLASS_INT){
                    a.num=-a.num;
                }else if(a.class==CLASS_FLOAT){
                    a.numf=-a.numf;
                }else{
                    reportVMError(vm,"unsupported class for prefix operation \"-\".",curPart);
                }
            case OPCODE_INVERT:
                a=vm->stack.vals[vm->stack.count-1];
                if(a.class==CLASS_INT){
                    a.num=~a.num;
                }else{
                    reportVMError(vm,"unsupported class for prefix operation \"~\".",curPart);
                }
            case OPCODE_NOT:
                a=vm->stack.vals[vm->stack.count-1];
                if(a.class==CLASS_INT){
                    a.num=!a.num;
                }else if(a.class==CLASS_FLOAT){
                    a.numf=!a.numf;
                }else{
                    reportVMError(vm,"unsupported class for prefix operation \"!\".",curPart);
                }
            case OPCODE_LOAD_CONST:
                loadConst(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_LOAD_VAL:
                loadVal(vm,clist.vals[++i],curPart);
                break;
            default:
                sprintf(temp,"unknown operation code (%d).",opcode);
                reportVMError(vm,temp,curPart);
        }
    }
}