#include"normal.h"
void initParser(Parser*parser){
    parser->code=NULL;
    parser->fileName=NULL;
    parser->ptr=0;
    parser->line=1;
    LIST_INIT(parser->varlist,Variable);
}
void clistToString(char*text,CmdList clist){
    Cmd cmd;
    char paraCount=0;
    char temp[50];
    char dat;
    int num,n;
    text[0]='\0';
    for(int i=0;i<clist.count;i++){
        cmd=clist.vals[i];
        switch(cmd.handle){
            case HANDLE_MOV:
                strcat(text,"mov");
                paraCount=2;
                break;
            case HANDLE_ADD:
                strcat(text,"add");
                paraCount=2;
                break;
            case HANDLE_SUB:
                strcat(text,"sub");
                paraCount=2;
                break;
            case HANDLE_MUL:
                strcat(text,"mul");
                paraCount=2;
                break;
            case HANDLE_DIV:
                strcat(text,"div");
                paraCount=2;
                break;
            case HANDLE_FAC:
                strcat(text,"fac");
                paraCount=1;
                break;
            case HANDLE_PUSH:
                strcat(text,"push");
                paraCount=1;
                break;
            case HANDLE_POP:
                strcat(text,"pop");
                paraCount=1;
                break;
            case HANDLE_JMP:
                strcat(text,"jmp");
                paraCount=1;
                break;
            case HANDLE_JMPC:
                strcat(text,"jmpc");
                paraCount=1;
                break;
            case HANDLE_EQUAL:
                strcat(text,"equal");
                paraCount=2;
                break;
            case HANDLE_SUBS:
                strcat(text,"subs");
                paraCount=1;
                break;
            case HANDLE_SET:
                strcat(text,"set");
                paraCount=2;
                break;
            case HANDLE_GET:
                strcat(text,"get");
                paraCount=2;
                break;
            case HANDLE_PTR:
                strcat(text,"ptr");
                paraCount=2;
                break;
            case HANDLE_SFREE:
                strcat(text,"sfree");
                paraCount=1;
                break;
            case HANDLE_POPT:
                strcat(text,"popt");
                paraCount=2;
                break;
            case HANDLE_CAND:
                strcat(text,"cand");
                paraCount=2;
                break;
            case HANDLE_COR:
                strcat(text,"cor");
                paraCount=2;
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
        if(paraCount==0){
            strcat(text,"\n");
            continue;
        }
        if(cmd.ta==DATA_INTEGER){
            sprintf(temp," %d",cmd.a);
        }else if(cmd.ta==DATA_FLOAT){
            dat=cmd.a>>29;
            num=cmd.a&0x1FFFFFFF;
            n=1;
            while(dat--){
                n*=10;
            }
            sprintf(temp," %d.%d",num/n,num%n);
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
            }else if(cmd.a==REG_CF){
                strcpy(temp," cf");
            }else if(cmd.a==REG_NULL){
                strcpy(temp," null");
            }else{
                sprintf(temp," unknownReg(%d)",cmd.a);
            }
        }else{
            sprintf(temp," unknownType(%d)",cmd.ta);
        }
        strcat(text,temp);
        if(paraCount==1){
            strcat(text,"\n");
            continue;
        }
        if(cmd.tb==DATA_INTEGER){
            sprintf(temp,",%d",cmd.b);
        }else if(cmd.tb==DATA_FLOAT){
            dat=cmd.b>>29;
            num=cmd.b&0x1FFFFFFF;
            n=1;
            while(dat--){
                n*=10;
            }
            sprintf(temp,",%d.%d",num/n,num%n);
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
            }else if(cmd.b==REG_CF){
                strcpy(temp,",cf");
            }else if(cmd.b==REG_NULL){
                strcpy(temp,",null");
            }else{
                sprintf(temp,",unknownReg(%d)",cmd.b);
            }
        }else{
            sprintf(temp,",unknownType(%d)",cmd.tb);
        }
        strcat(text,temp);
        strcat(text,"\n");
    }
    strcat(text,"\0");
}
void vlistToString(char*text,VariableList vlist){
    Variable var;
    char temp[72];
    text[0]='\0';
    for(int i=0;i<vlist.count;i++){
        var=vlist.vals[i];
        sprintf(temp,"%d:name:%s,type:%d,size:%d\n",i,var.name,var.value.type,var.value.size);
        strcat(text,temp);
    }
    strcat(text,"\0");
}
void reportError(Parser*parser,char*msg){
    printf("%s:%d:  error:%s\n",parser->fileName,parser->line,msg);
    exit(1);
}
void reportWarning(Parser*parser,char*msg){
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