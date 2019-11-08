#include"vm.h"
void initVM(VM*vm,Parser parser){
    vm->parser=parser;
    LIST_INIT(vm->stack,Value)
    LIST_INIT(vm->var,Var)
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
#define EXE_OPT(opt,optf) \
    Value a,b;\
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
                    a.numF=a.numF opt b.num;\
                    break;\
                case CLASS_FLOAT:\
                    a.numf=a.numF opt b.numf;\
                    break;\
                default:\
                    reportVMError(vm,"unsupport float operation.",curPart);\
            }\
            break;\
        default:\
            
    }

void execute(VM*vm,intList clist){
    int opcode,curPart;
    char temp[50];
    for(int i=0;i<clist.count;i++){
        curPart=clist.vals[i++];
        opcode=clist.vals[i];
        switch(opcode){
            case OPCODE_NOP:
                break;
            
            default:
                sprintf(temp,"unknown operation code (%d).",opcode);
                reportVMError(vm,temp,curPart);
        }
    }
}