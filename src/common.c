#include"common.h"
void initClass(Parser*parser){
    Class class;
    for(int i=0;i<OPT_METHOD_COUNT;i++){
        class.optMethod[i].clist.count=0;
    }
    LIST_INIT(class.methods,Func)
    LIST_INIT(class.var,Name)

    class.name=(char*)malloc(6);
    strcpy(class.name,"class");
    LIST_ADD(parser->classList,Class,class)

    class.name=(char*)malloc(9);
    strcpy(class.name,"function");
    LIST_ADD(parser->classList,Class,class)

    class.name=(char*)malloc(4);
    strcpy(class.name,"int");
    LIST_ADD(parser->classList,Class,class)

    class.name=(char*)malloc(6);
    strcpy(class.name,"float");
    LIST_ADD(parser->classList,Class,class)

    class.name=(char*)malloc(7);
    strcpy(class.name,"string");
    LIST_ADD(parser->classList,Class,class)
}
void initParser(Parser*parser){
    parser->code=NULL;
    parser->fileName=NULL;
    parser->ptr=0;
    parser->line=1;
    parser->column=1;
    parser->curPart=0;
    LIST_INIT(parser->partList,Part)
    LIST_INIT(parser->clist,int)
    LIST_INIT(parser->classList,Class)
    LIST_INIT(parser->funcList,Func)
    LIST_INIT(parser->symList,Symbol)
    initClass(parser);
}
void clistToString(Parser parser,intList clist,char*text){
    int cmd;
    bool isArg,isSymbol;
    Symbol symbol;
    char temp[50];
    text[0]='\0';
    for(int i=0;i<clist.count;i++){
        cmd=clist.vals[i++];
        sprintf(temp,"%d(%d:%d):",i,parser.partList.vals[cmd].line,parser.partList.vals[cmd].column);
        strcat(text,temp);
        cmd=clist.vals[i];
        switch(cmd){
            case OPCODE_NOP:
                strcpy(temp,"NOP");
                isArg=false;
                break;
            case OPCODE_LOAD_CONST:
                strcpy(temp,"LOAD_CONST");
                isArg=true;
                isSymbol=true;
                break;
            case OPCODE_ADD:
                strcpy(temp,"ADD");
                isArg=false;
                break;
            case OPCODE_SUB:
                strcpy(temp,"SUB");
                isArg=false;
                break;
            case OPCODE_MUL:
                strcpy(temp,"MUL");
                isArg=false;
                break;
            case OPCODE_DIV:
                strcpy(temp,"DIV");
                isArg=false;
                break;
            case OPCODE_AND:
                strcpy(temp,"AND");
                isArg=false;
                break;
            case OPCODE_OR:
                strcpy(temp,"OR");
                isArg=false;
                break;
            case OPCODE_LEFT:
                strcpy(temp,"LEFT");
                isArg=false;
                break;
            case OPCODE_RIGHT:
                strcpy(temp,"RIGHT");
                isArg=false;
                break;
            case OPCODE_GTHAN:
                strcpy(temp,"GTHAN");
                isArg=false;
                break;
            case OPCODE_LTHAN:
                strcpy(temp,"LTHAN");
                isArg=false;
                break;
            case OPCODE_NOT_EQUAL:
                strcpy(temp,"NOT_EQUAL");
                isArg=false;
                break;
            case OPCODE_GTHAN_EQUAL:
                strcpy(temp,"GTHAN_EQUAL");
                isArg=false;
                break;
            case OPCODE_LTHAN_EQUAL:
                strcpy(temp,"LTHAN_EQUAL");
                isArg=false;
                break;
            case OPCODE_INVERT:
                strcpy(temp,"INVERT");
                isArg=false;
                break;
            case OPCODE_NOT:
                strcpy(temp,"NOT");
                isArg=false;
                break;
            case OPCODE_REM:
                strcpy(temp,"REM");
                isArg=false;
                break;
            case OPCODE_SUBS:
                strcpy(temp,"SUBS");
                isArg=false;
                break;
            case OPCODE_LOAD_ATTR:
                strcpy(temp,"LOAD_ATTR");
                isArg=true;
                isSymbol=true;
                break;
            case OPCODE_LOAD_INDEX:
                strcpy(temp,"LOAD_INDEX");
                isArg=false;
                break;
            case OPCODE_LOAD_VAL:
                strcpy(temp,"LOAD_VAL");
                isArg=true;
                isSymbol=true;
                break;
            case OPCODE_STORE_VAL:
                strcpy(temp,"STORE_VAL");
                isArg=true;
                isSymbol=true;
                break;
            case OPCODE_STORE_ATTR:
                strcpy(temp,"STORE_ATTR");
                isArg=true;
                isSymbol=true;
                break;
            case OPCODE_STORE_INDEX:
                strcpy(temp,"STORE_INDEX");
                isArg=false;
                break;
            case OPCODE_STACK_COPY:
                strcpy(temp,"STACK_COPY");
                isArg=true;
                isSymbol=false;
                break;
            case OPCODE_PUSH_VAL:
                strcpy(temp,"PUSH_VAL");
                isArg=true;
                isSymbol=true;
                break;
            case OPCODE_POP_VAR:
                strcpy(temp,"POP_VAR");
                isArg=true;
                isSymbol=false;
                break;
            case OPCODE_POP_STACK:
                strcpy(temp,"POP_STACK");
                isArg=true;
                isSymbol=false;
                break;
            case OPCODE_JUMP:
                strcpy(temp,"JUMP");
                isArg=true;
                isSymbol=false;
                break;
            case OPCODE_JUMP_IF_FALSE:
                strcpy(temp,"JUMP_IF_FALSE");
                isArg=true;
                isSymbol=false;
                break;
            case OPCODE_SET_FIELD:
                strcpy(temp,"SET_FIELD");
                isArg=false;
                break;
            case OPCODE_FREE_FIELD:
                strcpy(temp,"FREE_FIELD");
                isArg=false;
                break;
            case OPCODE_SET_WHILE:
                strcpy(temp,"SET_WHILE");
                isArg=false;
                break;
            case OPCODE_FREE_WHILE:
                strcpy(temp,"FREE_WHILE");
                isArg=false;
                break;
            case OPCODE_CALL_FUNCTION:
                strcpy(temp,"CALL_FUNCTION");
                isArg=true;
                isSymbol=false;
                break;
            case OPCODE_CALL_METHOD:
                strcpy(temp,"CALL_METHOD");
                isArg=true;
                isSymbol=false;
                break;
            case OPCODE_RETURN:
                strcpy(temp,"RETURN");
                isArg=false;
                break;
            case OPCODE_ENABLE_FUNCTION:
                strcpy(temp,"ENABLE_FUNCTION");
                isArg=true;
                isSymbol=false;
                break;
            case OPCODE_MAKE_OBJECT:
                strcpy(temp,"MAKE_OBJECT");
                isArg=true;
                isSymbol=false;
                break;
            case OPCODE_EXTEND_CLASS:
                strcpy(temp,"EXTEND_CLASS");
                isArg=true;
                isSymbol=false;
                break;
            case OPCODE_LOAD_CLASS:
                strcpy(temp,"LOAD_CLASS");
                isArg=true;
                isSymbol=false;
                break;
            case OPCODE_LOAD_FUNCTION:
                strcpy(temp,"LOAD_FUNCTION");
                isArg=true;
                isSymbol=false;
                break;
            default:
                sprintf(temp,"UNKNOWN(%d)",cmd);
                isArg=false;
                break;
        }
        strcat(text,temp);
        if(isArg){
            cmd=clist.vals[++i];
            sprintf(temp," %d",cmd);
            strcat(text,temp);
            if(isSymbol){
                symbol=parser.symList.vals[cmd];
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
        clistToString(parser,func.clist,temp);
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
            printf("asdssds%d:%s\n",class.var.count,class.var.vals[i2]);
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
void reportMsg(Parser*parser,Msg msg){
    char*text,temp[10];
    int len=msg.end-msg.start;
    if(msg.type==MSG_ERROR){
        strcpy(temp,"error");
    }else{
        strcpy(temp,"warning");
    }
    if(len>0){
        text=(char*)malloc(len+1);
        for(int i=0;i<len;i++){
            text[i]=parser->code[i+msg.start];
        }
        text[len]='\0';
        printf("%s:%d:%d:  %s:\n     %s\n%s\n",parser->fileName,msg.line,msg.column,temp,text,msg.text);
    }else{
        printf("%s:%d:%d:  %s:%s\n",parser->fileName,msg.line,msg.column,temp,msg.text);
    }
    if(msg.type==MSG_ERROR){
        exit(1);
    }
}
void reportError(Parser*parser,char*text,int start){
    Msg msg;
    msg.type=MSG_ERROR;
    msg.line=parser->line;
    msg.column=parser->column;
    strcpy(msg.text,text);
    msg.start=start;
    msg.end=parser->ptr;
    reportMsg(parser,msg);
}
void reportWarning(Parser*parser,char*text,int start){
    Msg msg;
    msg.type=MSG_WARNING;
    msg.line=parser->line;
    msg.column=parser->column;
    strcpy(msg.text,text);
    msg.start=start;
    msg.end=parser->ptr;
    reportMsg(parser,msg);
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