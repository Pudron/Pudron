#include"common.h"
/*严格按照索引顺序排列*/
const OpcodeMsg opcodeList[]={
    {OPCODE_NOP,"NOP",false,false},
    {OPCODE_ADD,"ADD",false,false},
    {OPCODE_SUB,"SUB",false,false},
    {OPCODE_MUL,"MUL",false,false},
    {OPCODE_DIV,"DIV",false,false},
    {OPCODE_AND,"AND",false,false},
    {OPCODE_OR,"OR",false,false},
    {OPCODE_CAND,"CAND",false,false},
    {OPCODE_COR,"COR",false,false},
    {OPCODE_LEFT,"LEFT",false,false},
    {OPCODE_RIGHT,"RIGHT",false,false},
    {OPCODE_EQUAL,"EQUAL",false,false},
    {OPCODE_GTHAN,"GTHAN",false,false},
    {OPCODE_LTHAN,"LTHAN",false,false},
    {OPCODE_NOT_EQUAL,"NOT_EQUAL",false,false},
    {OPCODE_GTHAN_EQUAL,"GTHAN_EQUAL",false,false},
    {OPCODE_LTHAN_EQUAL,"LTHAN_EQUAL",false,false},
    {OPCODE_REM,"REM",false,false},
    {OPCODE_INVERT,"INVERT",false,false},
    {OPCODE_NOT,"NOT",false,false},
    {OPCODE_SUBS,"SUBS",false,false},
    {OPCODE_LOAD_CONST,"LOAD_CONST",true,true},
    {OPCODE_LOAD_VAL,"LOAD_VAL",true,true},
    {OPCODE_LOAD_ATTR,"LOAD_ATTR",true,true},
    {OPCODE_LOAD_INDEX,"LOAD_INDEX",false,false},
    {OPCODE_STORE_VAL,"STORE_VAL",true,true},
    {OPCODE_STORE_ATTR,"STORE_ATTR",true,true},
    {OPCODE_STORE_INDEX,"STORE_INDEX",false,false},
    {OPCODE_PUSH_VAL,"PUSH_VAL",true,true},
    {OPCODE_STACK_COPY,"STACK_COPY",true,false},
    {OPCODE_POP_VAR,"POP_VAR",true,false},
    {OPCODE_POP_STACK,"POP_STACK",true,false},
    {OPCODE_JUMP,"JUMP",true,false},
    {OPCODE_JUMP_IF_FALSE,"JUMP_IF_FALSE",true,false},
    {OPCODE_SET_FIELD,"SET_FIELD",false,false},
    {OPCODE_FREE_FIELD,"FREE_FIELD",false,false},
    {OPCODE_SET_LOOP,"SET_LOOP",false,false},
    {OPCODE_FREE_LOOP,"FREE_LOOP",false,false},
    {OPCODE_CALL_FUNCTION,"CALL_FUNCTION",true,false},
    {OPCODE_CALL_METHOD,"CALL_METHOD",true,false},
    {OPCODE_RETURN,"RETURN",false,false},
    {OPCODE_ENABLE_FUNCTION,"ENABLE_FUNCTION",true,false},
    {OPCODE_MAKE_OBJECT,"MAKE_OBJECT",true,false},
    {OPCODE_EXTEND_CLASS,"EXTEND_CLASS",true,false},
    {OPCODE_ENABLE_CLASS,"ENABLE_CLASS",true,false},
    {OPCODE_SET_MODULE,"SET_MODULE",true,true},
    {OPCODE_RETURN_MODULE,"RETURN_MODULE",false,false},
    {OPCODE_PRINT_STACK,"PRINT_STACK",false,false},
    {OPCODE_PRINT_VAR,"PRINT_VAR",false,false}
};
void initParser(Parser*parser,bool isRoot){
    parser->code=NULL;
    parser->fileName=NULL;
    parser->ptr=0;
    parser->line=1;
    parser->column=1;
    parser->curPart=0;
    if(isRoot){
        LIST_INIT(parser->partList,Part)
        LIST_INIT(parser->clist,int)
        LIST_INIT(parser->classList,Class)
        LIST_INIT(parser->funcList,Func)
        LIST_INIT(parser->symList,Symbol)
        LIST_INIT(parser->moduleList,Module)
    }
}
void extend(Class*class,Class eclass){
    LIST_CONNECT(class->var,eclass.var,Name,0)
    class->varBase=eclass.varBase;
    LIST_CONNECT(class->methods,eclass.methods,Func,1)
    for(int i=0;i<OPT_METHOD_COUNT;i++){
        class->optMethod[i]=eclass.optMethod[i];
    }
}
void clistToString(Parser parser,intList clist,char*text,Module module){
    int cmd;
    Symbol symbol;
    char temp[50];
    text[0]='\0';
    for(int i=0;i<clist.count;i++){
        cmd=clist.vals[i++];
        sprintf(temp,"%d(%d:%d):",i,parser.partList.vals[cmd+module.partBase].line,parser.partList.vals[cmd+module.partBase].column);
        strcat(text,temp);
        cmd=clist.vals[i];
        OpcodeMsg opcode=opcodeList[cmd];
        strcat(text,opcode.name);
        strcat(text," ");
        if(opcode.isArg){
            cmd=clist.vals[++i];
            sprintf(temp," %d",cmd);
            strcat(text,temp);
            if(opcode.isSymbol){
                symbol=parser.symList.vals[cmd+module.symBase];
                switch(symbol.type){
                    case SYM_INT:
                        sprintf(temp,"(%d)",symbol.num);
                        break;
                    case SYM_FLOAT:
                        sprintf(temp,"(%f)",symbol.numf);
                        break;
                    case SYM_STRING:
                        sprintf(temp,"(%s)",symbol.str);
                        break;
                    default:
                        sprintf(temp,"(unkown type:%d)",symbol.type);
                        break;
                }
                strcat(text,temp);
            }
        }
        strcat(text,"\n");
    }
}
void funcToString(Parser parser,FuncList funcList,char*text){
    text[0]='\0';
    Func func;
    char temp[500];
    for(int i=0;i<funcList.count;i++){
        func=funcList.vals[i];
        sprintf(temp,"Function %s(",func.name);
        strcat(text,temp);
        for(int i2=0;i2<func.args.count;i2++){
            strcat(text,func.args.vals[i2]);
            if(i2!=func.args.count-1){
                strcat(text,",");
            }
        }
        strcat(text,"):\n");
        clistToString(parser,func.clist,temp,parser.moduleList.vals[func.moduleID]);
        strcat(text,temp);
        strcat(text,"Function End\n\n");
    }
}
void classToString(Parser parser,char*text){
    text[0]='\0';
    Class class;
    char temp[500];
    for(int i=0;i<parser.classList.count;i++){
        class=parser.classList.vals[i];
        sprintf(temp,"Class %s:\n",class.name);
        strcat(text,temp);
        for(int i2=0;i2<class.var.count;i2++){
            strcat(text,class.var.vals[i2]);
            if(i2!=class.var.count-1){
                strcat(text,",");
            }else{
                strcat(text,";\n");
            }
        }
        funcToString(parser,class.methods,temp);
        strcat(text,temp);
        strcat(text,"Class End\n\n");
    }
}
char*cutText(char*text,int start,int end){
    char*str;
    int len=end-start;
    if(len<=0){
        return NULL;
    }
    str=(char*)malloc(len+1);
    for(int i=0;i<len;i++){
        str[i]=text[i+start];
    }
    str[len]='\0';
    return str;
}
void reportMsg(Msg msg){
    char*text,temp[10];
    text=cutText(msg.code,msg.start,msg.end);
    if(msg.type==MSG_ERROR){
        strcpy(temp,"error");
    }else{
        strcpy(temp,"warning");
    }
    if(text!=NULL){
        printf("%s:%d:%d:  %s:\n     %s\n%s\n",msg.fileName,msg.line,msg.column,temp,text,msg.text);
        free(text);
    }else{
        printf("%s:%d:%d:  %s:%s\n",msg.fileName,msg.line,msg.column,temp,msg.text);
    }
    if(msg.type==MSG_ERROR){
        exit(1);
    }
}
void reportError(Parser*parser,char*text,int start){
    Msg msg;
    msg.code=parser->code;
    msg.fileName=parser->fileName;
    msg.type=MSG_ERROR;
    msg.line=parser->line;
    msg.column=parser->column;
    strcpy(msg.text,text);
    msg.start=start;
    msg.end=parser->ptr;
    reportMsg(msg);
}
void reportWarning(Parser*parser,char*text,int start){
    Msg msg;
    msg.code=parser->code;
    msg.fileName=parser->fileName;
    msg.type=MSG_WARNING;
    msg.line=parser->line;
    msg.column=parser->column;
    strcpy(msg.text,text);
    msg.start=start;
    msg.end=parser->ptr;
    reportMsg(msg);
}
void addCmd(Parser*parser,intList*clist,int opcode){
    LIST_ADD((*clist),int,parser->curPart);
    LIST_ADD((*clist),int,opcode);
}
void addCmd1(Parser*parser,intList*clist,int opcode,int dat){
    LIST_ADD((*clist),int,parser->curPart);
    LIST_ADD((*clist),int,opcode);
    LIST_ADD((*clist),int,dat);
}
void addCmds(Parser*parser,intList*clist,Command cmds){
    LIST_ADD((*clist),int,parser->curPart);
    for(int i=0;i<cmds.count;i++){
        LIST_ADD((*clist),int,cmds.code[i])
    }
}
int addSymbol(Parser*parser,Symbol symbol){
    Symbol sym;
    for(int i=0;i<parser->symList.count;i++){
        sym=parser->symList.vals[i];
        if(symbol.type==SYM_INT && sym.type==SYM_INT){
            if(symbol.num==sym.num){
                return i;
            }
        }else if(symbol.type==SYM_FLOAT && sym.type==SYM_FLOAT){
            if(symbol.numf==sym.numf){
                return i;
            }
        }else if(symbol.type==SYM_STRING && sym.type==SYM_STRING){
            if(strcmp(sym.str,symbol.str)==0){
                free(symbol.str);
                return i;
            }
        }
    }
    LIST_ADD(parser->symList,Symbol,symbol)
    return parser->symList.count-1;
}