#include"normal.h"
int initParser(Parser*parser){
    ClassType ct;
    parser->code=NULL;
    parser->fileName=NULL;
    parser->ptr=0;
    parser->line=1;
    initCommands(&parser->cmds);
    LIST_INIT(parser->clist,Cmd);
    parser->clist.memory=0;
    LIST_INIT(parser->vlist,Value);
    LIST_INIT(parser->classList,ClassType);
    strcpy(ct.name,"char");
    ct.size=1;
    LIST_ADD(parser->classList,ClassType,ct);
    strcpy(ct.name,"num");
    ct.size=5;
    LIST_ADD(parser->classList,ClassType,ct);
    return 0;
}
int initCommands(Commands*cmds){
	cmds->vals=(unsigned char*)malloc(100);
	cmds->memory=100;
	cmds->count=0;
    return 0;
}
int addCmd(Commands*cmds,unsigned char c){
    cmds->count+=1;
    if(cmds->count>=cmds->memory){
        cmds->memory+=50;
        cmds->vals=(unsigned char*)realloc(cmds->vals,cmds->memory);
    }
    cmds->vals[cmds->count-1]=c;
    cmds->vals[cmds->count]='\0';
    return 0;
}
int addCmdInt(Commands*cmds,int c){
    cmds->count+=4;
    if(cmds->count>=cmds->memory){
        cmds->memory+=50;
        cmds->vals=(unsigned char*)realloc(cmds->vals,cmds->memory);
    }
    cmds->vals[cmds->count-4]=c>>24;
    cmds->vals[cmds->count-3]=c>>16;
    cmds->vals[cmds->count-2]=c>>8;
    cmds->vals[cmds->count-1]=c;
    cmds->vals[cmds->count]='\0';
    return 0;
}
int addCmdDats(Commands*cmds,char count,char val){
    int rp=cmds->count;
    cmds->count+=count;
    if(cmds->count>=cmds->memory){
        cmds->memory+=50+count;
        cmds->vals=(unsigned char*)realloc(cmds->vals,cmds->memory);
    }
    for(int i=rp;i<cmds->count;i++){
        cmds->vals[i]=val;
    }
    cmds->vals[cmds->count]='\0';
    return 0;
}
int addCmd1(Commands*cmds,Cmd cmd){
    unsigned char c;
    c=cmd.handle;
    addCmd(cmds,c);
    c=cmd.ta;
    addCmd(cmds,c);
    if(cmd.ta==DATA_INTEGER || cmd.ta==DATA_POINTER){
        addCmdInt(cmds,cmd.a);
    }else if(cmd.ta==DATA_REG || cmd.ta==DATA_CHAR){
        addCmd(cmds,cmd.a);
    }else{
        addCmd(cmds,cmd.a);
    }
    return 0;
}
int addCmd2(Commands*cmds,Cmd cmd){
    unsigned char c;
    c=cmd.handle;
    addCmd(cmds,c);
    c=(cmd.ta<<4)|cmd.tb;
    addCmd(cmds,c);
    if(cmd.ta==DATA_INTEGER || cmd.ta==DATA_POINTER){
        addCmdInt(cmds,cmd.a);
    }else if(cmd.ta==DATA_REG || cmd.ta==DATA_CHAR){
        addCmd(cmds,cmd.a);
    }else{
        addCmd(cmds,cmd.a);
    }
    if(cmd.tb==DATA_INTEGER || cmd.tb==DATA_POINTER){
        addCmdInt(cmds,cmd.a);
    }else if(cmd.tb==DATA_REG || cmd.ta==DATA_CHAR){
        addCmd(cmds,cmd.b);
    }else{
        addCmd(cmds,cmd.b);
    }
    return 0;
}
int getPara1(char*text,Commands cmds,int*ptr){
    char word[20];
    unsigned char ta;
    int ra=0;
    unsigned char c=cmds.vals[*ptr];
    ta=c;
    *ptr+=1;
    if(ta==DATA_INTEGER){
        ra=(cmds.vals[*ptr]<<24)|(cmds.vals[*ptr+1]<<16)|(cmds.vals[*ptr+2]<<8)|cmds.vals[*ptr+3];
        *ptr+=4;
        sprintf(word," %d\n",ra);
    }else if(ta==DATA_POINTER){
        ra=(cmds.vals[*ptr]<<24)|(cmds.vals[*ptr+1]<<16)|(cmds.vals[*ptr+2]<<8)|cmds.vals[*ptr+3];
        *ptr+=4;
        sprintf(word," [%d]\n",ra);
    }else if(ta==DATA_CHAR){
        ra=cmds.vals[*ptr];
        *ptr+=1;
        sprintf(word," %d\n",ra);
    }else if(ta==DATA_REG){
        ra=cmds.vals[*ptr];
        *ptr+=1;
        if(ra==REG_AX){
            strcpy(word," ax\n");
        }else if(ra==REG_BX){
            strcpy(word," bx\n");
        }else if(ra==REG_CX){
            strcpy(word," cx\n");
        }else if(ra==REG_DX){
            strcpy(word," dx\n");
        }else{
            sprintf(word,"AUnknown reg:%d\n",ra);
            strcat(text,word);
            *ptr+=1;
        }
    }else{
        sprintf(word,"Data1 error:%d\n",ta);
        strcat(text,word);
    }
    strcat(text,word);
    return 0;
}
int getPara2(char*text,Commands cmds,int*ptr){
    char word[20];
    unsigned char ta,tb;
    int ra=0,rb=0;
    unsigned char c=cmds.vals[*ptr];
    ta=c>>4;
    tb=c&0x0F;
    *ptr+=1;
    if(ta==DATA_INTEGER){
        ra=(cmds.vals[*ptr]<<24)|(cmds.vals[*ptr+1]<<16)|(cmds.vals[*ptr+2]<<8)|cmds.vals[*ptr+3];
        *ptr+=4;
        sprintf(word," %d",ra);
    }else if(ta==DATA_POINTER){
        ra=(cmds.vals[*ptr]<<24)|(cmds.vals[*ptr+1]<<16)|(cmds.vals[*ptr+2]<<8)|cmds.vals[*ptr+3];
        *ptr+=4;
        sprintf(word," [%d]",ra);
    }else if(ta==DATA_CHAR){
        ra=cmds.vals[*ptr];
        *ptr+=1;
        sprintf(word," %d",ra);
    }else if(ta==DATA_REG){
        ra=cmds.vals[*ptr];
        *ptr+=1;
        if(ra==REG_AX){
            strcpy(word," ax");
        }else if(ra==REG_BX){
            strcpy(word," bx");
        }else if(ra==REG_CX){
            strcpy(word," cx");
        }else if(ra==REG_DX){
            strcpy(word," dx");
        }else{
            sprintf(word," errorReg(%d)",ra);
            *ptr+=1;
        }
    }else{
        sprintf(word,"errorData(%d)",ta);
    }
    strcat(text,word);
    if(tb==DATA_INTEGER){
        rb=(cmds.vals[*ptr]<<24)|(cmds.vals[*ptr+1]<<16)|(cmds.vals[*ptr+2]<<8)|cmds.vals[*ptr+3];
        *ptr+=4;
        sprintf(word,",%d\n",rb);
    }else if(tb==DATA_POINTER){
        rb=(cmds.vals[*ptr]<<24)|(cmds.vals[*ptr+1]<<16)|(cmds.vals[*ptr+2]<<8)|cmds.vals[*ptr+3];
        *ptr+=4;
        sprintf(word,",[%d]\n",rb);
    }else if(tb==DATA_CHAR){
        rb=cmds.vals[*ptr];
        *ptr+=1;
        sprintf(word," %d\n",rb);
    }else if(tb==DATA_REG){
        rb=cmds.vals[*ptr];
        *ptr+=1;
        if(rb==REG_AX){
            strcpy(word,",ax\n");
        }else if(rb==REG_BX){
            strcpy(word,",bx\n");
        }else if(rb==REG_CX){
            strcpy(word,",cx\n");
        }else if(rb==REG_DX){
            strcpy(word,",dx\n");
        }else{
            sprintf(word,",errorReg(%d)\n",rb);
            *ptr+=1;
        }
    }else{
        sprintf(word,",errorData(%d)\n",tb);
    }
    strcat(text,word);
    return 0;
}
int cmdToString(char*text,Commands cmds){
    int ptr=0;
    char c=cmds.vals[0];
    text[0]='\0';
    while(ptr<cmds.count){
        if(c==HANDLE_MOV){
            strcat(text,"mov");
            ptr++;
            getPara2(text,cmds,&ptr);
        }if(c==HANDLE_MOVI){
            strcat(text,"movi");
            ptr++;
            getPara2(text,cmds,&ptr);
        }else if(c==HANDLE_ADD){
            strcat(text,"add");
            ptr++;
            getPara2(text,cmds,&ptr);
        }else if(c==HANDLE_SUB){
            strcat(text,"sub");
            ptr++;
            getPara2(text,cmds,&ptr);
        }else if(c==HANDLE_SUBS){
            strcat(text,"subs");
            ptr++;
            getPara1(text,cmds,&ptr);
        }else if(c==HANDLE_MUL){
            strcat(text,"mul");
            ptr++;
            getPara2(text,cmds,&ptr);
        }else if(c==HANDLE_DIV){
            strcat(text,"div");
            ptr++;
            getPara2(text,cmds,&ptr);
        }else if(c==HANDLE_EQUAL){
            strcat(text,"equal");
            ptr++;
            getPara2(text,cmds,&ptr);
        }else if(c==HANDLE_PUSH){
            strcat(text,"push");
            ptr++;
            getPara1(text,cmds,&ptr);
        }else if(c==HANDLE_PUSHI){
            strcat(text,"pushi");
            ptr++;
            getPara1(text,cmds,&ptr);
        }else if(c==HANDLE_POP){
            strcat(text,"pop");
            ptr++;
            getPara1(text,cmds,&ptr);
        }else if(c==HANDLE_POPI){
            strcat(text,"popi");
            ptr++;
            getPara1(text,cmds,&ptr);
        }else if(c==HANDLE_NOP){
            strcat(text,"nop\n");
            ptr++;
        }else if(c==HANDLE_ADDF){
            strcat(text,"addf\n");
            ptr++;
        }else if(c==HANDLE_SUBF){
            strcat(text,"subf\n");
            ptr++;
        }else if(c==HANDLE_MULF){
            strcat(text,"mulf\n");
            ptr++;
        }else if(c==HANDLE_DIVF){
            strcat(text,"divf\n");
            ptr++;
        }else{
            strcat(text,"unknown\n");
            printf("uknow:%d\n",c);
            ptr++;
        }
        c=cmds.vals[ptr];
    }
    return 0;
}
int clistToCmds(Commands*cmds,CmdList clist){
    int ptr=0;
    unsigned char c;
    Cmd cmd;
    cmds->vals=(unsigned char*)malloc(clist.count+1);
    for(int i=0;i<clist.count;i++){
        cmd=clist.vals[i];
        cmds->vals[ptr++]=cmd.handle;
        if(cmd.parac==2){
            c=(cmd.ta<<4)|cmd.tb;
        }else{
            c=cmd.ta;
        }
        cmds->vals[ptr++]=c;
        if(cmd.ta==DATA_INTEGER || cmd.ta==DATA_POINTER){
            cmds->vals[ptr++]=cmd.a>>24;
            cmds->vals[ptr++]=cmd.a>>16;
            cmds->vals[ptr++]=cmd.a>>8;
            cmds->vals[ptr++]=cmd.a;
        }else{
            cmds->vals[ptr++]=cmd.a;
        }
        if(cmd.parac>=2){
            if(cmd.tb==DATA_INTEGER || cmd.tb==DATA_POINTER){
                cmds->vals[ptr++]=cmd.b>>24;
                cmds->vals[ptr++]=cmd.b>>16;
                cmds->vals[ptr++]=cmd.b>>8;
                cmds->vals[ptr++]=cmd.b;
            }else{
                cmds->vals[ptr++]=cmd.b;
            }
        }
    }
}
int clistToString(char*text,CmdList clist){

}