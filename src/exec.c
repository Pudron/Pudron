#include"exec.h"
/*void compile(Parser*parser){
    int rptr,rline;
    Token token;
    CmdList clist;
    Environment envirn;
    Cmd cmd;
    char text[1024];
    while(1){
        rptr=parser->ptr;
        rline=parser->line;
        token=nextToken(parser);
        if(token.type==TOKEN_END){
            break;
        }
        parser->ptr=rptr;
        parser->line=rline;
        LIST_INIT(clist,Cmd)
        if(getVariableDef(parser,&parser->varlist,&clist,envirn)){
            clistToString(text,clist);
            printf("VarDef Clist:\n%s\n",text);
        }else if(getAssignment(parser,&clist,envirn)){
            clistToString(text,clist);
            printf("Assignment Clist:\n%s\n",text);
        }else if(getConditionState(parser,&clist,&parser->varlist,envirn)){
            clistToString(text,clist);
            printf("ConditionState Clist:\n%s\n",text);
        }else if(getWhileLoop(parser,&clist,&parser->varlist,envirn)){
            clistToString(text,clist);
            printf("While Clist:\n%s\n",text);
        }else if(getExpression(parser,&clist,envirn)){
            token=nextToken(parser);
            if(token.type!=TOKEN_SEMI){
                reportError(parser,"expected \";\" after an expression");
            }
            cmd.handle=HANDLE_SFREE;
            cmd.ta=DATA_INTEGER;
            cmd.a=1;
            LIST_ADD(clist,Cmd,cmd);
            clistToString(text,clist);
            printf("Exp Clist:\n%s\n",text);
        }else{
            reportError(parser,"unknown expression.");
        }
        LIST_DELETE(clist);
    }
    vlistToString(text,parser->varlist);
    printf("VarList:\n%s\n",text);
}
void eMov(Parser*parser,Cmd cmd){
    Value value;
    char msg[80];
    value.size=0;
    if(cmd.tb==DATA_FLOAT){
        value.type=TYPE_FLOAT;
        value.val=cmd.b;
    }else if(cmd.tb==DATA_INTEGER){
        value.type=TYPE_INTEGER;
        value.val=cmd.b;
    }else if(cmd.tb==DATA_POINTER){
        value=parser->varlist.vals[cmd.b].value;
    }else if(cmd.tb==DATA_REG){
        value=parser->regs[cmd.b];
    }else{
        sprintf(msg,"execute:unknown tb %d.",cmd.tb);
        reportError(parser,msg);
    }
    if(cmd.ta==DATA_REG){
        parser->regs[cmd.a]=value;
    }else if(cmd.tb==DATA_POINTER){
        parser->varlist.vals[cmd.a].value=value;
    }else{
        sprintf(msg,"execute:unsupported ta %d.",cmd.ta);
        reportError(parser,msg);
    }
}
void ePtr(Parser*parser,Cmd cmd){
    char msg[50];
    if(cmd.ta==DATA_POINTER){
        parser->varlist.vals[cmd.a].value.type=TYPE_POINTER;
        parser->varlist.vals[cmd.a].value.val=cmd.b;
    }else if(cmd.ta==DATA_REG){
        parser->regs[cmd.a].type=TYPE_POINTER;
        parser->regs[cmd.a].val=cmd.b;
    }else{
        sprintf(msg,"execute:unsupported ta %d.",cmd.tb);
        reportError(parser,msg);
    }
}
void eSet(Parser*parser,Cmd cmd){
    Value value;
    char msg[80];
    value.size=0;
    if(cmd.tb==DATA_FLOAT){
        value.type=TYPE_FLOAT;
        value.val=cmd.b;
    }else if(cmd.tb==DATA_INTEGER){
        value.type=TYPE_INTEGER;
        value.val=cmd.b;
    }else if(cmd.tb==DATA_POINTER){
        value=parser->varlist.vals[cmd.b].value;
    }else if(cmd.tb==DATA_REG){
        value=parser->regs[cmd.b];
    }else{
        sprintf(msg,"execute:unknown tb %d.",cmd.tb);
        reportError(parser,msg);
    }
    if(cmd.ta==DATA_POINTER){
        Value v=parser->varlist.vals[cmd.a].value;
        if(v.type==TYPE_POINTER){
            parser->varlist.vals[v.val].value=value;
        }else if(v.type!=TYPE_FLOAT && v.type!=TYPE_INTEGER){
            *(v.ptr)=value;
        }
    }else if(cmd.ta==DATA_REG){
        Value v=parser->regs[cmd.a];
        if(v.type==TYPE_POINTER){
            parser->varlist.vals[v.val].value=value;
        }else if(v.type!=TYPE_FLOAT && v.type!=TYPE_INTEGER){
            *(v.ptr)=value;
        }
    }else{
        sprintf(msg,"execute:unsupported ta %d.",cmd.ta);
        reportError(parser,msg);
    }
}
void eGet(Parser*parser,Cmd cmd){
    Value value;
    if(parser->regs[cmd.b].type==TYPE_POINTER){
        value=parser->varlist.vals[parser->regs[cmd.b].val].value;
    }else{
        value=*(parser->regs[cmd.b].ptr);
    }
    if(cmd.ta==DATA_POINTER){
        parser->varlist.vals[cmd.a].value=value;
    }else{
        parser->regs[cmd.a]=value;
    }
}
void execute(Parser*parser,CmdList clist){
    Cmd cmd;
    for(int i=0;i<clist.count;i++){
        cmd=clist.vals[i];
        switch(cmd.handle){
            case HANDLE_NOP:
                break;
            case HANDLE_MOV:
                eMov(parser,cmd);
                break;
            case HANDLE_PTR:
                ePtr(parser,cmd);
                break;
            case HANDLE_SET:
                eSet(parser,cmd);
                break;
            case HANDLE_GET:
                eGet(parser,cmd);
                break;
            default:
                printf("Execute error:unknown command %d.\n",cmd.handle);
                break;
        }
    }
}*/
void exeError(char*text){
    printf("Execute error:%s\n",text);
    exit(2);
}
int getCmdData(VM*vm,DataType dt,int val){
    char msg[100];
    if(dt==DATA_INTEGER){
        return val;
    }else if(dt==DATA_POINTER){
        return vm->data[val];
    }else if(dt==DATA_REG){
        return vm->reg[val];
    }else if(dt==DATA_REG_POINTER){
        return vm->data[vm->reg[val]];
    }else{
        sprintf(msg,"unsupport data type:%d.",val);
        exeError(msg);
    }
    return 0;
}
int*getCmdPtr(VM*vm,DataType dt,int val){
    char msg[100];
    if(dt==DATA_POINTER){
        return &vm->data[val];
    }else if(dt==DATA_REG){
        return &vm->reg[val];
    }else if(dt==DATA_REG_POINTER){
        return &vm->data[vm->reg[val]];
    }else{
        sprintf(msg,"unsupport data type:%d.",val);
        exeError(msg);
    }
    return NULL;
}
void eNop(){
    return;
}
void eMov(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)=getCmdData(vm,cmd.tb,cmd.b);
}
void eAdd(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)+=getCmdData(vm,cmd.tb,cmd.b);
}
void eSub(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)-=getCmdData(vm,cmd.tb,cmd.b);
}
void eMul(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)*=getCmdData(vm,cmd.tb,cmd.b);
}
void eDiv(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)/=getCmdData(vm,cmd.tb,cmd.b);
}
/*a的cf次方 */
int myPow(int a,int cf){
    int result=1;
    while(cf--){
        result*=a;
    }
    return result;
}
void eFadd(VM*vm,Cmd cmd){
    
}


