#include"common.h"
void initParser(Parser*parser){
    parser->code=NULL;
    parser->fileName=NULL;
    parser->ptr=0;
    parser->line=1;
    parser->column=1;
    parser->curPart=0;
    LIST_INIT(parser->plist,int)
    LIST_INIT(parser->partList,PartMsg)
    LIST_INIT(parser->clist,int)
    LIST_INIT(parser->classList,Class)
    LIST_INIT(parser->symList,Symbol)
}
void clistToString(Parser parser,intList clist,char*text){
    int cmd;
    bool isArg,isSymbol;
    Symbol symbol;
    char temp[50];
    text[0]='\0';
    for(int i=0;i<clist.count;i++){
        cmd=clist.vals[i];
        sprintf(temp,"%d:",i);
        strcat(text,temp);
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
            case OPCODE_POP_VAL:
                strcpy(temp,"POP_VAL");
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
            case OPCODE_SET_VALS:
                strcpy(temp,"SET_VALS");
                isArg=false;
                break;
            case OPCODE_FREE_VALS:
                strcpy(temp,"FREE_VALS");
                isArg=false;
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
    LIST_ADD((*clist),int,opcode);
    LIST_ADD(parser->plist,int,parser->curPart)
}
void addCmd1(Parser*parser,intList*clist,int opcode,int dat){
    LIST_ADD((*clist),int,opcode);
    LIST_ADD((*clist),int,dat);
    LIST_ADD(parser->plist,int,parser->curPart)
    LIST_ADD(parser->plist,int,parser->curPart)
}
void addCmds(Parser*parser,intList*clist,Command cmds){
    for(int i=0;i<cmds.count;i++){
        LIST_ADD((*clist),int,cmds.code[i])
        LIST_ADD(parser->plist,int,parser->curPart)
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