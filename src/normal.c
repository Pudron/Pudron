#include"normal.h"
void initParser(Parser*parser){
    ClassType ct;
    parser->code=NULL;
    parser->fileName=NULL;
    parser->ptr=0;
    parser->line=1;
    LIST_INIT(parser->varlist,Variable);
    LIST_INIT(parser->classList,ClassType);
}
/*int clistToString(char*text,CmdList clist){
    Cmd cmd;
    char temp[20];
    text[0]='\0';
    for(int i=0;i<clist.count;i++){
        cmd=clist.vals[i];
        switch(cmd.handle){
            case HANDLE_MOV:
                strcat(text,"mov");
                break;
            case HANDLE_MOVI:
                strcat(text,"movi");
                break;
            case HANDLE_ADD:
                strcat(text,"add");
                break;
            case HANDLE_SUB:
                strcat(text,"sub");
                break;
            case HANDLE_MUL:
                strcat(text,"mul");
                break;
            case HANDLE_DIV:
                strcat(text,"div");
                break;
            case HANDLE_ADDF:
                strcat(text,"addf");
                break;
            case HANDLE_SUBF:
                strcat(text,"subf");
                break;
            case HANDLE_MULF:
                strcat(text,"mulf");
                break;
            case HANDLE_DIVF:
                strcat(text,"divf");
                break;
            case HANDLE_PUSH:
                strcat(text,"push");
                break;
            case HANDLE_PUSHI:
                strcat(text,"pushi");
                break;
            case HANDLE_POP:
                strcat(text,"pop");
                break;
            case HANDLE_POPI:
                strcat(text,"popi");
                break;
            case HANDLE_JMP:
                strcat(text,"jmp");
                break;
            case HANDLE_EQUAL:
                strcat(text,"equa;");
                break;
            case HANDLE_SUBS:
                strcat(text,"subs");
                break;
            case HANDLE_NOP:
                strcat(text,"nop\n");
                continue;
                break;
            default:
                sprintf(temp,"unknown:%d\n",cmd.handle);
                strcat(text,temp);
                continue;
                break;
        }
        if(cmd.parac==0){
            strcat(text,"\n");
            continue;
        }
        if(cmd.ta==DATA_CHAR || cmd.ta==DATA_INTEGER){
            sprintf(temp," %d",cmd.a);
        }else if(cmd.ta==DATA_POINTER){
            sprintf(temp," [%d]",cmd.a);
        }else if(cmd.ta==DATA_REG){
            if(cmd.a==REG_AX){
                strcpy(temp," ax");
            }else if(cmd.a==REG_BX){
                strcpy(temp," bx");
            }else if(cmd.a==REG_CX){
                strcpy(temp," cx");
            }else if(cmd.a==REG_DX){
                strcpy(temp," dx");
            }else{
                sprintf(temp," unknownReg(%d)",cmd.a);
            }
        }else{
            sprintf(temp," unknownType(%d)",cmd.ta);
        }
        strcat(text,temp);
        if(cmd.parac==1){
            strcat(text,"\n");
            continue;
        }
        if(cmd.tb==DATA_CHAR || cmd.tb==DATA_INTEGER){
            sprintf(temp,",%d",cmd.b);
        }else if(cmd.tb==DATA_POINTER){
            sprintf(temp,",[%d]",cmd.b);
        }else if(cmd.tb==DATA_REG){
            if(cmd.b==REG_AX){
                strcpy(temp,",ax");
            }else if(cmd.b==REG_BX){
                strcpy(temp,",bx");
            }else if(cmd.b==REG_CX){
                strcpy(temp,",cx");
            }else if(cmd.b==REG_DX){
                strcpy(temp,",dx");
            }else{
                sprintf(temp,",unknownReg(%d)",cmd.b);
            }
        }else{
            sprintf(temp,",unknownType(%d)",cmd.tb);
        }
        strcat(text,temp);
        if(cmd.parac==2){
            strcat(text,"\n");
            continue;
        }
    }
    return 0;
}*/
void reportError(Parser*parser,char*msg){
    printf("%s:%d:  error:%s\n",parser->fileName,parser->line,msg);
    exit(1);
}
void reprotWarning(Parser*parser,char*msg){
    printf("%s:%d:  warning:%s\n",parser->fileName,parser->line,msg);
}
void connectCmdList(CmdList*clist,CmdList newClist){
    int start=clist->count;
    clist->count+=newClist.count;
    clist->vals=(Cmd*)realloc(clist->vals,sizeof(Cmd)*clist->count);
    for(int i=0;i<newClist.count;i++){
        clist->vals[start+i]=newClist.vals[i];
    }
}