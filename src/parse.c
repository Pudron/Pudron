#include"parse.h"
Token nextToken(Parser*parser){
    char msgt[50];
    Token token;
    char c=parser->code[parser->ptr];
    while(c==' ' || c=='\n' || c=='\''){
        if(c=='\n'){
            parser->line++;
        }
        if(c=='\''){
            /*跳过注释*/
            c=parser->code[++parser->ptr];
            while(c!='\''){
                if(c=='\0'){
                    reportError(parser,"unterminated comment.\n");
                }
                if(c=='\n'){
                    parser->line++;
                }
                c=parser->code[++parser->ptr];
            }
            c=parser->code[++parser->ptr];
        }
        c=parser->code[++parser->ptr];
    }
    if(c=='\0'){
        token.type=TOKEN_END;
        return token;
    }
    if(c>='0' && c<='9'){
        int num=0,dat=0;
        while(c>='0' && c<='9'){
            num=num*10+c-'0';
            c=parser->code[++parser->ptr];
        }
        token.type=TOKEN_INTEGER;
        token.num=num;
        if(c=='.'){
            c=parser->code[++parser->ptr];
            while(c>='0' && c<='9'){
                dat++;
                num=num*10+c-'0';
                c=parser->code[++parser->ptr];
            }
            if(dat==0){
                sprintf(msgt,"\n    %d.\nexpected decimal.\n",num);
                reportError(parser,msgt);
            }
            if(dat>7){
                reportError(parser,"the length of the decimal is too long.\n");
            }
            token.type=TOKEN_FLOAT;
            token.num=(num&0x1FFFFFFF)|(dat<<29);
        }
        return token;
    }else if((c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_' || (c>='0' && c<='9')){
        int i=0;
        for(i=0;(c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_' || (c>='0' && c<='9');i++){
            if(i>=WORD_MAX-1){
                token.word[i]='\0';
                token.type=TOKEN_UNKNOWN;
                sprintf(msgt,"\n    %s...\ntoo long word.\n",token.word);
                reportError(parser,msgt);
            }
            token.word[i]=c;
            c=parser->code[++parser->ptr];
        }
        token.word[i+1]='\0';
        if(strcmp(token.word,"var")==0){
            token.type=TOKEN_VAR;
        }else if(strcmp(token.word,"func")==0){
            token.type=TOKEN_FUNC;
        }else if(strcmp(token.word,"array")==0){
            token.type=TOKEN_ARRAY;
        }else if(strcmp(token.word,"if")==0){
            token.type=TOKEN_IF;
        }else if(strcmp(token.word,"elif")==0){
            token.type=TOKEN_ELIF;
        }else if(strcmp(token.word,"else")==0){
            token.type=TOKEN_ELSE;
        }else{
            token.type=TOKEN_WORD;
        }
        return token;
    }else if(c=='+'){
        token.type=TOKEN_ADD;
        parser->ptr++;
        return token;
    }else if(c=='-'){
        token.type=TOKEN_SUB;
        parser->ptr++;
        return token;
    }else if(c=='*'){
        token.type=TOKEN_MUL;
        parser->ptr++;
        return token;
    }else if(c=='/'){
        token.type=TOKEN_DIV;
        parser->ptr++;
        return token;
    }else if(c=='('){
        token.type=TOKEN_PARE1;
        parser->ptr++;
        return token;
    }else if(c==')'){
        token.type=TOKEN_PARE2;
        parser->ptr++;
        return token;
    }else if(c=='{'){
        token.type=TOKEN_BRACE1;
        parser->ptr++;
        return token;
    }else if(c=='}'){
        token.type=TOKEN_BRACE2;
        parser->ptr++;
        return token;
    }else if(c=='.'){
        token.type=TOKEN_COMMA;
        parser->ptr++;
        return token;
    }else if(c==';'){
        token.type=TOKEN_SEMI;
        parser->ptr++;
        return token;
    }else if(c=='='){
        token.type=TOKEN_EQUAL;
        parser->ptr++;
        return token;
    }else if(c=='!'){
        token.type=TOKEN_EXCL;
        parser->ptr++;
        return token;
    }else if(c=='>'){
        if(parser->code[parser->ptr+1]=='='){
            token.type=TOKEN_GTHAN_EQUAL;
            parser->ptr+=2;
            return token;
        }
        token.type=TOKEN_GTHAN;
        parser->ptr++;
        return token;
    }else if(c=='<'){
        if(parser->code[parser->ptr+1]=='='){
            token.type=TOKEN_LTHAN_EQUAL;
            parser->ptr+=2;
            return token;
        }
        token.type=TOKEN_LTHAN;
        parser->ptr++;
        return token;
    }else{
        sprintf(msgt,"unknown charactor \"%c\".\n",c);
        reportError(parser,msgt);
        parser->ptr++;
    }
    return token;
}
bool getExpression(Parser*parser,CmdList*clist){
    Token token;
    OperatList olist;
    Operat operat;
    Cmd cmd;
    int rptr;
    bool isEnd=false;
    LIST_INIT(olist,Operat);
    while(1){
        rptr=parser->ptr;
        token=nextToken(parser);
        /*处理前缀运算*/
        if(token.type==TOKEN_SUB){
            operat.handle_prefix=HANDLE_SUBS;
            token=nextToken(parser);
        }else{
            operat.handle_prefix=HANDLE_NOP;
        }
        if(token.type==TOKEN_PARE1){
            getExpression(parser,clist);
            token=nextToken(parser);
            if(token.type!=TOKEN_PARE2){
                reportError(parser,"expected \")\".\n");
            }
        }else if(token.type==TOKEN_INTEGER || token.type==TOKEN_FLOAT){
            cmd.handle=HANDLE_PUSH;
            cmd.ta=DATA_INTEGER;
            cmd.a=token.num;
            LIST_ADD((*clist),Cmd,cmd);
            operat.type=token.type==TOKEN_FLOAT?DATA_FLOAT:DATA_INTEGER;
        }else{
            parser->ptr=rptr;
            return false;
        }
        rptr=parser->ptr;
        token=nextToken(parser);
        /*处理后缀运算*/
        if(token.type==TOKEN_EXCL){
            operat.handle_postfix=HANDLE_FAC;
            token=nextToken(parser);
        }else{
            operat.handle_prefix=HANDLE_NOP;
        }
        /*处理中间运算*/
        if(token.type==TOKEN_ADD){
            operat.handle_infix=HANDLE_ADD;
            operat.power=50;
        }else if(token.type==TOKEN_SUB){
            operat.handle_infix=HANDLE_SUB;
            operat.power=50;
        }else if(token.type==TOKEN_MUL){
            operat.handle_infix=HANDLE_MUL;
            operat.power=80;
        }else if(token.type==TOKEN_DIV){
            operat.handle_infix=HANDLE_DIV;
            operat.power=80;
        }else{
            /*表达式结束*/
            parser->ptr=rptr;
            isEnd=1;
        }
        if(olist.count>=1){
            if(olist.vals[olist.count-1].power>=operat.power || isEnd){
                cmd.handle=HANDLE_POP;
                cmd.ta=DATA_REG;
                cmd.tb=DATA_REG;
                cmd.a=REG_BX;
                LIST_ADD((*clist),Cmd,cmd);
                if(operat.handle_postfix!=HANDLE_NOP){
                    cmd.handle=operat.handle_postfix;
                    LIST_ADD((*clist),Cmd,cmd);
                    operat.handle_postfix=HANDLE_NOP;
                }
                if(operat.handle_prefix!=HANDLE_NOP){
                    cmd.handle=operat.handle_prefix;
                    LIST_ADD((*clist),Cmd,cmd);
                    operat.handle_prefix=HANDLE_NOP;
                }
            }
            while(olist.count>=1){
                Operat opt=olist.vals[olist.count-1];
                if(operat.power>opt.power && !isEnd){
                    break;
                }
                cmd.handle=HANDLE_POP;
                cmd.a=REG_AX;
                LIST_ADD((*clist),Cmd,cmd);
                if(opt.handle_postfix!=HANDLE_NOP){
                    cmd.handle=opt.handle_postfix;
                    LIST_ADD((*clist),Cmd,cmd);
                }
                if(opt.handle_prefix!=HANDLE_NOP){
                    cmd.handle=opt.handle_prefix;
                    LIST_ADD((*clist),Cmd,cmd);
                }
                cmd.handle=opt.handle_infix;
                cmd.b=REG_BX;
                LIST_ADD((*clist),Cmd,cmd);
                cmd.handle=HANDLE_MOV;
                cmd.a=REG_BX;
                cmd.b=REG_AX;
                LIST_ADD((*clist),Cmd,cmd);
                LIST_SUB(olist);
            }
            cmd.handle=HANDLE_PUSH;
            cmd.a=REG_AX;
            LIST_ADD((*clist),Cmd,cmd);
        }
        if(isEnd){
            break;
        }
        LIST_ADD(olist,Operat,operat);
    }
    LIST_DELETE(olist);
    return true;
}
Msg getValueGlobalDef(Parser*parser){
    Msg msg,msg2;
    Token token;
    Value value;
    Cmd cmd;
    int rptr=parser->ptr;
    msg.type=MSG_SUCCESS;
    msg2=nextToken(parser,&token);
    MSG_CHECK(msg,msg2);
    if(token.type==TOKEN_ARRAY){
        value.extype=EXTYPE_DYNAMIC_ARRAY;
        msg2=nextToken(parser,&token);
        MSG_CHECK(msg,msg2);
    }else{
        value.extype=EXTYPE_NORMAL;
    }
    value.type=-1;
    if(token.type==TOKEN_CHAR){
        value.type=TYPE_CHAR;
    }else if(token.type==TOKEN_NUM){
        value.type=TYPE_NUMBER;
    }else if(token.type==TOKEN_WORD){
        for(int i=2;i<parser->classList.count;i++){
            if(strcmp(parser->classList.vals[i].name,token.word)==0){
                value.type=i;
                break;
            }
        }
        if(value.type==-1){
            parser->ptr=rptr;
            msg.type=MSG_NONE;
            return msg;
        }
    }else{
        parser->ptr=rptr;
        msg.type=MSG_NONE;
        return msg;
    }
    while(1){
        msg2=nextToken(parser,&token);
        MSG_CHECK(msg,msg2);
        if(token.type!=TOKEN_WORD){
            sprintf(msg2.text,"%s:%d:error:expected a value name in defination.\n",parser->fileName,parser->line);
            msg.type=MSG_ERROR;
            strcat(msg.text,msg2.text);
        }
        strcpy(value.name,token.word);
        msg2=nextToken(parser,&token);
        MSG_CHECK(msg,msg2);
        LIST_ADD(parser->vlist,Value,value);
        value.ptr=parser->cmds.count;
        addCmdDats(&parser->cmds,parser->classList.vals[value.type].size,0);
        if(token.type==TOKEN_EQUAL){
            int rtype;
            msg2=getExpression(parser,&parser->clist,&rtype);
            MSG_CHECK(msg,msg2);
            if((value.type==TYPE_CHAR || value.type==TYPE_NUMBER) && (rtype==TYPE_CHAR || rtype==TYPE_NUMBER) && value.extype==EXTYPE_NORMAL){
                if(rtype==TYPE_NUMBER){
                    cmd.handle=HANDLE_POP;
                    cmd.ta=DATA_REG;
                    cmd.a=REG_CX;
                    cmd.parac=1;
                    LIST_ADD((parser->clist),Cmd,cmd);
                    cmd.handle=HANDLE_POPI;
                    cmd.a=REG_AX;
                    LIST_ADD((parser->clist),Cmd,cmd);
                    parser->clist.memory+=6;
                }else{
                    cmd.handle=HANDLE_POP;
                    cmd.ta=DATA_REG;
                    cmd.a=REG_AX;
                    cmd.parac=1;
                    LIST_ADD((parser->clist),Cmd,cmd);
                    parser->clist.memory+=3;
                }
                cmd.ta=DATA_POINTER;
                cmd.tb=DATA_REG;
                cmd.a=value.ptr;
                cmd.b=REG_AX;
                cmd.parac=2;
                if(value.type==TYPE_CHAR || rtype==TYPE_CHAR){
                    cmd.handle=HANDLE_MOV;
                    LIST_ADD((parser->clist),Cmd,cmd);
                    parser->clist.count+=7;
                }else if(value.type==TYPE_NUMBER){
                    cmd.handle=HANDLE_MOVI;
                    LIST_ADD((parser->clist),Cmd,cmd);
                    cmd.handle=HANDLE_MOV;
                    cmd.a=value.ptr+4;
                    cmd.b=REG_CX;
                    LIST_ADD((parser->clist),Cmd,cmd);
                    parser->clist.count+=14;
                }
            }else if(rtype!=value.type){
                msg.type=MSG_ERROR;
                sprintf(msg2.text,"%s:%d:error:can not initialize the value from the type \"%s\" to \"%s\".\n",parser->fileName,parser->line,parser->classList.vals[rtype].name,parser->classList.vals[value.type].name);
                strcat(msg.text,msg2.text);
            }else{
                /*对象赋值*/
            }
            msg2=nextToken(parser,&token);
            MSG_CHECK(msg,msg2);
        }
        if(token.type==TOKEN_COMMA){
            continue;
        }else if(token.type==TOKEN_SEMI){
            break;
        }else{
            msg.type=MSG_ERROR;
            sprintf(msg2.text,"%s:%d:error:expect \";\" or \",\".\n",parser->fileName,parser->line);
            strcat(msg.text,msg2.text);
            break;
        }
    }
    return msg;
}
Msg parse(Parser*parser){
    Msg msg,msg2;
    Token token;
    int rptr;
    msg.type=MSG_SUCCESS;
    while(1){
        rptr=parser->ptr;
        msg2=nextToken(parser,&token);
        MSG_CHECK(msg,msg2);
        if(token.type==TOKEN_END){
            break;
        }
        parser->ptr=rptr;
        msg2=getValueGlobalDef(parser);
        MSG_PARSE_CHECK(msg,msg2);
        msg.type=MSG_ERROR;
        sprintf(msg2.text,"%s:%d:error:unknown expression.\n",parser->fileName,parser->line);
        strcat(msg.text,msg2.text);
        break;
    }
    return msg;
}