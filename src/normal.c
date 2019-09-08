#include"normal.h"
void initParser(Parser*parser){
    parser->code=NULL;
    parser->fileName=NULL;
    parser->ptr=0;
    parser->line=1;
    parser->dataSize=0;
    LIST_INIT(parser->varlist,Variable);
    LIST_INIT(parser->classList,ClassType);
    LIST_INIT(parser->exeClist,Cmd);
    ClassType classType;
    classType.size=1;
    strcpy(classType.name,"int");
    LIST_ADD(parser->classList,ClassType,classType);
    strcpy(classType.name,"float");
    LIST_ADD(parser->classList,ClassType,classType);
}
void clistToString(char*text,CmdList clist,bool isNum){
    Cmd cmd;
    char paraCount=0;
    char temp[50];
    text[0]='\0';
    for(int i=0;i<clist.count;i++){
        cmd=clist.vals[i];
        if(isNum){
            sprintf(temp,"%d.",i);
            strcat(text,temp);
        }
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
            case HANDLE_FADD:
                strcat(text,"fadd");
                paraCount=2;
                break;
            case HANDLE_FSUB:
                strcat(text,"fsub");
                paraCount=2;
                break;
            case HANDLE_FMUL:
                strcat(text,"fmul");
                paraCount=2;
                break;
            case HANDLE_FDIV:
                strcat(text,"fdiv");
                paraCount=2;
                break;
            case HANDLE_GTHAN:
                strcat(text,"gthan");
                paraCount=2;
                break;
            case HANDLE_GTHAN_EQUAL:
                strcat(text,"gthanEqual");
                paraCount=2;
                break;
            case HANDLE_LTHAN:
                strcat(text,"lthan");
                paraCount=2;
                break;
            case HANDLE_LTHAN_EQUAL:
                strcat(text,"lthanEqual");
                paraCount=2;
                break;
            case HANDLE_NOT_EQUAL:
                strcat(text,"notEqual");
                paraCount=2;
                break;
            case HANDLE_INVERT:
                strcat(text,"invert");
                paraCount=1;
                break;
            case HANDLE_AND:
                strcat(text,"and");
                paraCount=2;
                break;
            case HANDLE_OR:
                strcat(text,"or");
                paraCount=2;
                break;
            case HANDLE_LEFT:
                strcat(text,"left");
                paraCount=2;
                break;
            case HANDLE_RIGHT:
                strcat(text,"right");
                paraCount=2;
                break;
            case HANDLE_INVERT2:
                strcat(text,"invert2");
                paraCount=1;
                break;
            case HANDLE_FTOI:
                strcat(text,"ftoi");
                paraCount=2;
                break;
            case HANDLE_PUTC:
                strcat(text,"putc");
                paraCount=1;
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
        }else if(cmd.ta==DATA_REG_POINTER){
            if(cmd.a==REG_AX){
                strcpy(temp," [ax]");
            }else if(cmd.a==REG_BX){
                strcpy(temp," [bx]");
            }else if(cmd.a==REG_CX){
                strcpy(temp," [cx]");
            }else if(cmd.a==REG_DX){
                strcpy(temp," [dx]");
            }else if(cmd.a==REG_CF){
                strcpy(temp," [cf]");
            }else if(cmd.a==REG_NULL){
                strcpy(temp," [null]");
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
        }else if(cmd.tb==DATA_REG_POINTER){
            if(cmd.b==REG_AX){
                strcpy(temp,",[ax]");
            }else if(cmd.b==REG_BX){
                strcpy(temp,",[bx]");
            }else if(cmd.b==REG_CX){
                strcpy(temp,",[cx]");
            }else if(cmd.b==REG_DX){
                strcpy(temp,",[dx]");
            }else if(cmd.b==REG_CF){
                strcpy(temp,",[cf]");
            }else if(cmd.b==REG_NULL){
                strcpy(temp,",[null]");
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
    Variable var,var2;
    char temp[100];
    text[0]='\0';
    for(int i=0;i<vlist.count;i++){
        var=vlist.vals[i];
        sprintf(temp,"%d:name:%s,class:%d,ptr:%d,dim:%d\n",i,var.name,var.class,var.ptr,var.dim);
        strcat(text,temp);
        if(var.dim){
            strcat(text,"array:");
            var2=var;
            while(var2.dim){
                sprintf(temp,"%d ",var2.arrayCount);
                strcat(text,temp);
                var2=*var2.subVar;
            }
            strcat(text,"\n");
        }
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
    clist->size+=newClist.count;
    clist->vals=(Cmd*)realloc(clist->vals,sizeof(Cmd)*clist->count);
    for(int i=0;i<newClist.count;i++){
        clist->vals[start+i]=newClist.vals[i];
    }
}
void addCmd1(CmdList*clist,HandleType ht,DataType ta,int a){
    Cmd cmd;
    cmd.handle=ht;
    cmd.ta=ta;
    cmd.a=a;
    LIST_ADD((*clist),Cmd,cmd);
}
void addCmd2(CmdList*clist,HandleType ht,DataType ta,DataType tb,int a,int b){
    Cmd cmd;
    cmd.handle=ht;
    cmd.ta=ta;
    cmd.tb=tb;
    cmd.a=a;
    cmd.b=b;
    LIST_ADD((*clist),Cmd,cmd);
}