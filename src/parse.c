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
                    reportError(parser,"unterminated comment.");
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
                sprintf(msgt,"\n    %d.\nexpected decimal.",num);
                reportError(parser,msgt);
            }
            if(dat>7){
                reportError(parser,"the length of the decimal is too long.");
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
                sprintf(msgt,"\n    %s...\ntoo long word.",token.word);
                reportError(parser,msgt);
            }
            token.word[i]=c;
            c=parser->code[++parser->ptr];
        }
        token.word[i]='\0';
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
    }else if(c==','){
        token.type=TOKEN_COMMA;
        parser->ptr++;
        return token;
    }else if(c=='.'){
        token.type=TOKEN_POINT;
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
        sprintf(msgt,"unknown charactor \"%c\".",c);
        reportError(parser,msgt);
        parser->ptr++;
    }
    return token;
}
bool getExpression(Parser*parser,CmdList*clist,Environment envirn){
    Token token;
    OperatList olist;
    Operat operat;
    Cmd cmd;
    char msg[50];
    int rptr,rline;
    bool isEnd=false;
    LIST_INIT(olist,Operat);
    while(1){
        rptr=parser->ptr;
        rline=parser->line;
        token=nextToken(parser);
        /*处理前缀运算*/
        if(token.type==TOKEN_SUB){
            operat.handle_prefix=HANDLE_SUBS;
            token=nextToken(parser);
        }else{
            operat.handle_prefix=HANDLE_NOP;
        }
        if(token.type==TOKEN_PARE1){
            if(!getExpression(parser,clist,envirn)){
                reportWarning(parser,"expected an expression in the \"( )\".");
            }
            token=nextToken(parser);
            if(token.type!=TOKEN_PARE2){
                reportError(parser,"expected \")\".");
            }
        }else if(token.type==TOKEN_INTEGER || token.type==TOKEN_FLOAT){
            cmd.handle=HANDLE_PUSH;
            cmd.ta=(token.type==TOKEN_FLOAT)?DATA_FLOAT:DATA_INTEGER;
            cmd.a=token.num;
            LIST_ADD((*clist),Cmd,cmd);
        }else if(token.type==TOKEN_WORD){
            if(!getVarRef(parser,token.word,clist,&cmd,envirn)){
                sprintf(msg,"unfound variable \"%s\".",token.word);
                reportError(parser,msg);
            }
            cmd.handle=HANDLE_PUSH;
            cmd.ta=DATA_REG;
            cmd.a=REG_AX;
            LIST_ADD((*clist),Cmd,cmd);
        }else{
            parser->ptr=rptr;
            parser->line=rline;
            return false;
        }
        rptr=parser->ptr;
        rline=parser->line;
        token=nextToken(parser);
        /*处理后缀运算*/
        if(token.type==TOKEN_EXCL){
            rptr=parser->ptr;
            rline=parser->line;
            operat.handle_postfix=HANDLE_FAC;
            token=nextToken(parser);
        }else{
            operat.handle_postfix=HANDLE_NOP;
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
            parser->line=rline;
            isEnd=1;
        }
        if(olist.count>=1){
            bool isRight=false;
            if(olist.vals[olist.count-1].power>=operat.power || isEnd){
                isRight=true;
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
                LIST_SUB(olist,Operat);
            }
            if(isRight){
                cmd.handle=HANDLE_PUSH;
                cmd.a=REG_AX;
                LIST_ADD((*clist),Cmd,cmd);
            }
        }
        if(isEnd){
            if(olist.count==0 && (operat.handle_postfix!=HANDLE_NOP || operat.handle_prefix!=HANDLE_NOP)){
                cmd.handle=HANDLE_POP;
                cmd.ta=DATA_REG;
                cmd.a=REG_AX;
                LIST_ADD((*clist),Cmd,cmd);
                if(operat.handle_postfix!=HANDLE_NOP){
                    cmd.handle=operat.handle_postfix;
                    LIST_ADD((*clist),Cmd,cmd);
                }
                if(operat.handle_prefix!=HANDLE_NOP){
                    cmd.handle=operat.handle_prefix;
                    LIST_ADD((*clist),Cmd,cmd);
                }
                cmd.handle=HANDLE_PUSH;
                LIST_ADD((*clist),Cmd,cmd);
            }
            break;
        }
        LIST_ADD(olist,Operat,operat);
    }
    LIST_DELETE(olist);
    return true;
}
bool getVariableDef(Parser*parser,VariableList*vlist,CmdList*clist,Environment envirn){
    Token token;
    Variable var;
    Cmd cmd;
    int rptr=parser->ptr;
    int rline=parser->line;
    token=nextToken(parser);
    if(token.type!=TOKEN_VAR){
        parser->ptr=rptr;
        parser->line=rline;
        return false;
    }
    while(1){
        token=nextToken(parser);
        if(token.type!=TOKEN_WORD){
            reportError(parser,"expected a variable name.");
        }
        strcpy(var.name,token.word);
        var.value.size=0;
        var.value.type=TYPE_INTEGER;
        var.value.dat.val=0;
        LIST_ADD((*vlist),Variable,var);
        token=nextToken(parser);
        if(token.type==TOKEN_EQUAL){
            if(!getExpression(parser,clist,envirn)){
                reportError(parser,"expected an expression when initializing variable");
            }
            cmd.handle=HANDLE_POP;
            cmd.ta=DATA_REG;
            cmd.a=REG_AX;
            LIST_ADD((*clist),Cmd,cmd);
            cmd.handle=HANDLE_MOV;
            cmd.ta=DATA_POINTER;
            cmd.tb=DATA_REG;
            cmd.a=vlist->count-1;
            cmd.b=REG_AX;
            LIST_ADD((*clist),Cmd,cmd);
            token=nextToken(parser);
        }
        if(token.type==TOKEN_COMMA){
            continue;
        }else if(token.type==TOKEN_SEMI){
            break;
        }else{
            reportError(parser,"expected \",\" or \";\" after initializing variable");
        }
    }
    return true;
}
bool getAssignment(Parser*parser,CmdList*clist,Environment envirn){
    AsCmdsList alist;
    Token token;
    AsCmds ascmds;
    Cmd cmd;
    int rptr,rline;
    rptr=parser->ptr;
    rline=parser->line;
    LIST_INIT(alist,AsCmds);
    while(1){
        token=nextToken(parser);
        if(token.type!=TOKEN_WORD){
            parser->ptr=rptr;
            parser->line=rline;
            return false;
        }
        LIST_INIT(ascmds.clist,Cmd);
        if(!getVarRef(parser,token.word,&ascmds.clist,&ascmds.asCmd,envirn)){
            parser->ptr=rptr;
            parser->line=rline;
            LIST_DELETE(ascmds.clist);
            for(int i=0;i<alist.count;i++){
                LIST_DELETE(alist.vals[i].clist);
            }
            LIST_DELETE(alist);
            return false;
        }
        LIST_ADD(alist,AsCmds,ascmds);
        token=nextToken(parser);
        if(token.type==TOKEN_COMMA){
            continue;
        }else if(token.type==TOKEN_EQUAL){
            break;
        }else{
            parser->ptr=rptr;
            parser->line=rline;
            for(int i=0;i<alist.count;i++){
                LIST_DELETE(alist.vals[i].clist);
            }
            LIST_DELETE(alist);
            return false;
        }
    }
    int i=-1;
    while(1){
        ascmds=alist.vals[++i];
        if(i>=alist.count){
            reportError(parser,"too many assignment.");
        }
        if(!getExpression(parser,clist,envirn)){
            reportError(parser,"expected an expression in assignment.");
        }
        connectCmdList(clist,ascmds.clist);
        cmd.handle=HANDLE_POP;
        cmd.ta=DATA_REG;
        cmd.a=REG_BX;
        LIST_ADD((*clist),Cmd,cmd);
        ascmds.asCmd.tb=DATA_REG;
        ascmds.asCmd.b=REG_BX;
        LIST_ADD((*clist),Cmd,ascmds.asCmd);
        token=nextToken(parser);
        if(token.type==TOKEN_COMMA){
            continue;
        }else if(token.type==TOKEN_SEMI){
            if(i<alist.count-1){
                for(int i2=i+1;i2<alist.count;i2++){
                    ascmds=alist.vals[i2];
                    connectCmdList(clist,ascmds.clist);
                    cmd.handle=HANDLE_POP;
                    cmd.ta=DATA_REG;
                    cmd.a=REG_BX;
                    LIST_ADD((*clist),Cmd,cmd);
                    ascmds.asCmd.tb=DATA_REG;
                    ascmds.asCmd.b=REG_BX;
                    LIST_ADD((*clist),Cmd,ascmds.asCmd);
                    cmd.handle=HANDLE_PUSH;
                    LIST_ADD((*clist),Cmd,cmd);
                }
            }
            for(int i2=0;i2<alist.count;i2++){
                LIST_DELETE(alist.vals[i2].clist);
            }
            LIST_DELETE(alist);
            break;
        }else{
            reportError(parser,"expected \",\" or \";\" after assignment.");
        }
    }
    return true;
}
/*不止这么简单的getVarRef()*/
bool getVarRef(Parser*parser,char*varName,CmdList*clist,Cmd*asCmd,Environment envirn){
    Cmd cmd;
    for(int i=0;i<parser->varlist.count;i++){
        if(strcmp(varName,parser->varlist.vals[i].name)==0){
            asCmd->handle=HANDLE_MOV;
            asCmd->ta=DATA_POINTER;
            asCmd->a=i;
            cmd.handle=HANDLE_MOV;
            cmd.ta=DATA_REG;
            cmd.tb=DATA_POINTER;
            cmd.a=REG_AX;
            cmd.b=i;
            LIST_ADD((*clist),Cmd,cmd);
            return true;
        }
    }
    return false;
}