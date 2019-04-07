#include"parse.h"
Msg nextToken(Parser*parser,Token*token){
    Msg msg;
    char msgt[50];
    char c=parser->code[parser->ptr];
    msg.type=MSG_SUCCESS;
    while(c==' ' || c=='\n' || c=='\''){
        if(c=='\n'){
            parser->line++;
        }
        if(c=='\''){
            /*跳过注释*/
            c=parser->code[++parser->ptr];
            while(c!='\''){
                if(c=='\0'){
                    token->type=TOKEN_UNKNOWN;
                    msg.type=MSG_ERROR_MUST;
                    sprintf(msgt,"%s:%d:error:unterminated comment.\n",parser->fileName,parser->line);
                    strcat(msg.text,msgt);
                    return msg;
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
        token->type=TOKEN_END;
        return msg;
    }
    if(c>='0' && c<='9'){
        int num=0,dat=0;
        while(c>='0' && c<='9'){
            num=num*10+c-'0';
            c=parser->code[++parser->ptr];
        }
        if(c=='.'){
            c=parser->code[++parser->ptr];
            while(c>='0' && c<='9'){
                dat++;
                num=num*10+c-'0';
                c=parser->code[++parser->ptr];
            }
            if(dat==0){
                token->type=TOKEN_UNKNOWN;
                msg.type=MSG_ERROR;
                sprintf(msgt,"%s:%d:error:\n    %d.\nexpected decimal.\n",parser->fileName,parser->line,num);
                strcat(msg.text,msgt);
            }
        }
        token->type=TOKEN_NUMBER;
        token->num=num;
        token->dat=dat;
        return msg;
    }else if((c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_' || (c>='0' && c<='9')){
        int i=0;
        for(i=0;(c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_' || (c>='0' && c<='9');i++){
            if(i>=WORD_MAX-1){
                token->word[i]='\0';
                msg.type=MSG_ERROR_MUST;
                token->type=TOKEN_UNKNOWN;
                sprintf(msgt,"%s:%d:error:\n    %s...\ntoo long word.\n",parser->fileName,parser->line,token->word);
                strcat(msg.text,msgt);
                parser->ptr++;
                return msg;
            }
            token->word[i]=c;
            c=parser->code[++parser->ptr];
        }
        token->word[i+1]='\0';
        if(strcmp(token->word,"char")==0){
            token->type=TOKEN_CHAR;
        }else if(strcmp(token->word,"num")==0){
            token->type=TOKEN_NUM;
        }else if(strcmp(token->word,"array")==0){
            token->type=TOKEN_ARRAY;
        }else if(strcmp(token->word,"static")==0){
            token->type=TOKEN_STATIC;
        }else{
            token->type=TOKEN_WORD;
        }
        return msg;
    }else if(c=='+'){
        token->type=TOKEN_ADD;
        parser->ptr++;
        return msg;
    }else if(c=='-'){
        token->type=TOKEN_SUB;
        parser->ptr++;
        return msg;
    }else if(c=='*'){
        token->type=TOKEN_MUL;
        parser->ptr++;
        return msg;
    }else if(c=='/'){
        token->type=TOKEN_DIV;
        parser->ptr++;
        return msg;
    }else if(c=='('){
        token->type=TOKEN_PARE1;
        parser->ptr++;
        return msg;
    }else if(c==')'){
        token->type=TOKEN_PARE2;
        parser->ptr++;
        return msg;
    }else if(c=='{'){
        token->type=TOKEN_BRACE1;
        parser->ptr++;
        return msg;
    }else if(c=='}'){
        token->type=TOKEN_BRACE2;
        parser->ptr++;
        return msg;
    }else if(c=='.'){
        token->type=TOKEN_COMMA;
        parser->ptr++;
        return msg;
    }else if(c==';'){
        token->type=TOKEN_SEMI;
        parser->ptr++;
        return msg;
    }else if(c=='='){
        token->type=TOKEN_EQUAL;
        parser->ptr++;
        return msg;
    }else if(c=='>'){
        if(parser->code[parser->ptr+1]=='='){
            token->type=TOKEN_GTHAN_EQUAL;
            parser->ptr+=2;
            return msg;
        }
        token->type=TOKEN_GTHAN;
        parser->ptr++;
        return msg;
    }else if(c=='<'){
        if(parser->code[parser->ptr+1]=='='){
            token->type=TOKEN_LTHAN_EQUAL;
            parser->ptr+=2;
            return msg;
        }
        token->type=TOKEN_LTHAN;
        parser->ptr++;
        return msg;
    }else{
        msg.type=MSG_ERROR;
        token->type=TOKEN_UNKNOWN;
        sprintf(msgt,"%s:%d:error:unknown charactor \"%c\".\n",parser->fileName,parser->line,c);
        strcat(msg.text,msgt);
        parser->ptr++;
    }
    return msg;
}
Msg getExpression(Parser*parser,CmdList*clist,int*rtype){
    Msg msg,msg2;
    Token token;
    OperatList olist;
    Operat operat;
    int rptr;
    char isFirst=1;
    char isPrefix=1;
    char isEnd=0;
    Cmd cmd;
    cmd.isWeak=0;
    msg.type=MSG_SUCCESS;
    LIST_INIT(olist,Operat);
    while(1){
        msg2=nextToken(parser,&token);
        MSG_CHECK(msg,msg2);
        if(isPrefix){
            /*处理单目运算*/
            if(token.type==TOKEN_SUB){
                operat.handle=HANDLE_SUBS;
                msg2=nextToken(parser,&token);
                MSG_CHECK(msg,msg2);
            }else{
                isPrefix=0;
            }
        }
        if(token.type==TOKEN_PARE1){
            msg2=getExpression(parser,clist,&operat.ltype);
            MSG_CHECK(msg,msg2);
            if(olist.count>=1){
                olist.vals[olist.count-1].rtype=operat.ltype;
            }
            msg2=nextToken(parser,&token);
            MSG_CHECK(msg,msg2);
            if(token.type!=TOKEN_PARE2){
                sprintf(msg2.text,"%s:%d:error:expected \")\".\n",parser->fileName,parser->line);
                msg.type=MSG_ERROR;
                strcat(msg.text,msg2.text);
            }
        }else if(token.type==TOKEN_NUMBER){
            cmd.handle=HANDLE_PUSHI;
            cmd.ta=DATA_INTEGER;
            cmd.a=token.num;
            cmd.parac=1;
            LIST_ADD((*clist),Cmd,cmd);
            cmd.handle=HANDLE_PUSH;
            cmd.ta=DATA_CHAR;
            cmd.a=token.dat;
            LIST_ADD((*clist),Cmd,cmd);
            clist->memory+=6;
            operat.ltype=TYPE_NUMBER;
            if(olist.count>=1){
                olist.vals[olist.count-1].rtype=operat.ltype;
            }
            operat.isFunc=0;
        }else{
            sprintf(msg2.text,"%s:%d:error:expected a value or constant in the expression.\n",parser->fileName,parser->line);
            msg.type=MSG_ERROR;
            strcat(msg.text,msg2.text);
        }
        if(isFirst){
            *rtype=operat.ltype;
            isFirst=0;
        }
        if(isPrefix){
            isPrefix=0;
            if(operat.ltype!=TYPE_CHAR && operat.ltype!=TYPE_NUMBER){
                msg.type=MSG_ERROR;
                sprintf(msg2.text,"%s:%d:error:the prefix operator only supports type num or char.\n",parser->fileName,parser->line);
                strcat(msg.text,msg2.text);
            }
            cmd.handle=HANDLE_POP;
            cmd.ta=DATA_REG;
            cmd.a=REG_CX;
            cmd.parac=1;
            LIST_ADD((*clist),Cmd,cmd);
            cmd.handle=HANDLE_POPI;
            cmd.a=REG_AX;
            LIST_ADD((*clist),Cmd,cmd);
            cmd.handle=operat.handle;
            cmd.parac=0;
            LIST_ADD((*clist),Cmd,cmd);
            cmd.parac=1;
            cmd.handle=HANDLE_PUSHI;
            LIST_ADD((*clist),Cmd,cmd);
            cmd.handle=HANDLE_PUSH;
            cmd.a=REG_CX;
            LIST_ADD((*clist),Cmd,cmd);
            clist->memory+=13;
        }
        rptr=parser->ptr;
        msg2=nextToken(parser,&token);
        MSG_CHECK(msg,msg2);
        if(token.type==TOKEN_ADD){
            operat.handle=HANDLE_ADDF;
            operat.power=50;
        }else if(token.type==TOKEN_SUB){
            operat.handle=HANDLE_SUBF;
            operat.power=50;
        }else if(token.type==TOKEN_MUL){
            operat.handle=HANDLE_MULF;
            operat.power=80;
        }else if(token.type==TOKEN_DIV){
            operat.handle=HANDLE_DIVF;
            operat.power=80;
        }else{
            /*表达式结束*/
            parser->ptr=rptr;
            isEnd=1;
        }
        while(olist.count>=1){
            Operat opt=olist.vals[olist.count-1];
            if(operat.power>opt.power && !isEnd){
                break;
            }
            if((opt.ltype==TYPE_CHAR || opt.ltype==TYPE_NUMBER) && (opt.rtype==TYPE_CHAR || opt.rtype==TYPE_NUMBER)){
                cmd.handle=HANDLE_POP;
                cmd.ta=DATA_REG;
                cmd.a=REG_DX;
                cmd.parac=1;
                LIST_ADD((*clist),Cmd,cmd);
                cmd.handle=HANDLE_POPI;
                cmd.a=REG_BX;
                LIST_ADD((*clist),Cmd,cmd);
                cmd.handle=HANDLE_POP;
                cmd.a=REG_CX;
                LIST_ADD((*clist),Cmd,cmd);
                cmd.handle=HANDLE_POPI;
                cmd.a=REG_AX;
                LIST_ADD((*clist),Cmd,cmd);
                cmd.parac=0;
                cmd.handle=opt.handle;
                LIST_ADD((*clist),Cmd,cmd);
                cmd.parac=1;
                cmd.handle=HANDLE_PUSHI;
                cmd.a=REG_AX;
                LIST_ADD((*clist),Cmd,cmd);
                cmd.handle=HANDLE_PUSH;
                cmd.a=REG_CX;
                LIST_ADD((*clist),Cmd,cmd);
                clist->memory+=19;
            }else{
                msg.type=MSG_ERROR;
                sprintf(msg2.text,"%s:%d:error:unknown operation for type \"%s\" and \"%s\".",parser->fileName,parser->line,parser->classList.vals[opt.ltype].name,parser->classList.vals[opt.rtype].name);
                strcat(msg.text,msg2.text);
            }
            olist.count--;
        }
        if(isEnd){
            break;
        }
        LIST_ADD(olist,Operat,operat);
    }
    LIST_DELETE(olist);
    return msg;
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
                cmd.handle=HANDLE_POP;
                cmd.ta=DATA_REG;
                cmd.a=REG_CX;
                cmd.parac=1;
                LIST_ADD((parser->clist),Cmd,cmd);
                cmd.handle=HANDLE_POPI;
                cmd.a=REG_AX;
                LIST_ADD((parser->clist),Cmd,cmd);
                cmd.ta=DATA_POINTER;
                cmd.tb=DATA_REG;
                cmd.a=value.ptr;
                cmd.b=REG_AX;
                cmd.parac=2;
                if(value.type==TYPE_CHAR){
                    cmd.handle=HANDLE_MOV;
                    LIST_ADD((parser->clist),Cmd,cmd);
                    parser->clist.count+=10;
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