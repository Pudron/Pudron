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
        }else if(strcmp(token.word,"while")==0){
            token.type=TOKEN_WHILE;
        }else if(strcmp(token.word,"if")==0){
            token.type=TOKEN_IF;
        }else if(strcmp(token.word,"elif")==0){
            token.type=TOKEN_ELIF;
        }else if(strcmp(token.word,"else")==0){
            token.type=TOKEN_ELSE;
        }else if(strcmp(token.word,"and")==0){
            token.type=TOKEN_CAND;
        }else if(strcmp(token.word,"or")==0){
            token.type=TOKEN_COR;
        }else{
            token.type=TOKEN_WORD;
        }
        return token;
    }else if(c=='+'){
        if(parser->code[parser->ptr+1]=='+'){
            token.type=TOKEN_DOUBLE_ADD;
            parser->ptr+=2;
            return token;
        }
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
            operat.handle_prefix=HANDLE_SUB;
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
            operat.type=OPT_MIX;
        }else if(token.type==TOKEN_INTEGER || token.type==TOKEN_FLOAT){
            cmd.handle=HANDLE_PUSH;
            cmd.ta=DATA_INTEGER;
            cmd.a=token.num;
            LIST_ADD((*clist),Cmd,cmd);
            operat.type=(token.type==TOKEN_INTEGER)?OPT_INTEGER:OPT_FLOAT;
        }else if(token.type==TOKEN_WORD){
            if(!getVarRef(parser,token.word,clist,envirn)){
                sprintf(msg,"unfound variable \"%s\".",token.word);
                reportError(parser,msg);
            }
            cmd.handle=HANDLE_PUSH;
            cmd.ta=DATA_REG;
            cmd.b=REG_AX;
            LIST_ADD((*clist),Cmd,cmd);
            operat.type=OPT_POINTER;
        }else{
            parser->ptr=rptr;
            parser->line=rline;
            return false;
        }
        rptr=parser->ptr;
        rline=parser->line;
        token=nextToken(parser);
        /*处理后缀运算*/
        if(token.type==TOKEN_DOUBLE_ADD){
            rptr=parser->ptr;
            rline=parser->line;
            operat.handle_postfix=HANDLE_ADD;
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
        }else if(token.type==TOKEN_EQUAL){
            operat.handle_infix=HANDLE_EQUAL;
            operat.power=30;
        }else if(token.type==TOKEN_CAND){
            operat.handle_infix=HANDLE_CAND;
            operat.power=10;
        }else if(token.type==TOKEN_COR){
            operat.handle_infix=HANDLE_COR;
            operat.power=10;
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
                cmd.a=REG_BX;
                LIST_ADD((*clist),Cmd,cmd);
                if(operat.handle_postfix==HANDLE_ADD){
                    if(operat.type==OPT_FLOAT){
                        cmd.handle=HANDLE_FADD;
                        cmd.tb=DATA_INTEGER;
                        cmd.b=1;
                        LIST_ADD((*clist),Cmd,cmd);
                    }else if(operat.type==OPT_INTEGER){
                        cmd.handle=HANDLE_ADD;
                        cmd.tb=DATA_INTEGER;
                        cmd.b=1;
                        LIST_ADD((*clist),Cmd,cmd);
                    }else if(operat.type==OPT_POINTER){
                        addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG,REG_CX,REG_BX);
                        addCmd2(clist,HANDLE_ADD,DATA_REG,DATA_INTEGER,REG_CX,1);
                        addCmd2(clist,HANDLE_EQUAL,DATA_REG_POINTER,DATA_INTEGER,REG_CX,TYPE_INTEGER);
                        addCmd1(clist,HANDLE_JMPC,DATA_INTEGER,3);
                        addCmd2(clist,HANDLE_ADD,DATA_REG_POINTER,DATA_INTEGER,REG_BX,1);
                        addCmd1(clist,HANDLE_JMP,DATA_INTEGER,4);
                        addCmd2(clist,HANDLE_EQUAL,DATA_REG_POINTER,DATA_INTEGER,REG_CX,TYPE_FLOAT);
                        addCmd1(clist,HANDLE_JMPC,DATA_INTEGER,2);
                        addCmd2(clist,HANDLE_FADD,DATA_REG_POINTER,DATA_INTEGER,REG_BX,1);
                    }else if(operat.type==OPT_MIX){
                        reportWarning(parser,"the expression do not support mixture postfix calculation.");
                    }
                    operat.handle_postfix=HANDLE_NOP;
                }
                if(operat.handle_prefix!=HANDLE_NOP){
                    if(operat.type==OPT_POINTER){
                        addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG_POINTER,REG_BX,REG_BX);
                        addCmd1(clist,operat.handle_prefix,DATA_REG,REG_BX);
                    }else{
                        addCmd1(clist,operat.handle_prefix,DATA_REG_POINTER,REG_BX);
                    }
                    operat.handle_prefix=HANDLE_NOP;
                }
                operat.type=OPT_MIX;
            }
            while(olist.count>=1){
                Operat opt=olist.vals[olist.count-1];
                if(operat.power>opt.power && !isEnd){
                    break;
                }
                cmd.handle=HANDLE_POP;
                cmd.ta=DATA_REG;
                cmd.tb=DATA_REG;
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
    char msg[60];
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
        for(int i=0;i<parser->varlist.count;i++){
            if(strcmp(token.word,parser->varlist.vals[i].name)==0){
                sprintf(msg,"the variable \"%s\" has already exist.",token.word);
                reportError(parser,msg);
            }
        }
        strcpy(var.name,token.word);
        var.value.size=0;
        var.value.type=TYPE_INTEGER;
        var.value.val=0;
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
    Token token;
    Cmd cmd;
    int rptr,rline;
    int stackPtr=-1;
    rptr=parser->ptr;
    rline=parser->line;
    while(1){
        token=nextToken(parser);
        if(token.type!=TOKEN_WORD){
            parser->ptr=rptr;
            parser->line=rline;
            return false;
        }
        if(!getVarRef(parser,token.word,clist,envirn)){
            parser->ptr=rptr;
            parser->line=rline;
            return false;
        }
        cmd.handle=HANDLE_PUSH;
        cmd.ta=DATA_REG;
        cmd.a=REG_AX;
        LIST_ADD((*clist),Cmd,cmd);
        token=nextToken(parser);
        stackPtr++;
        if(token.type==TOKEN_COMMA){
            continue;
        }else if(token.type==TOKEN_EQUAL){
            break;
        }else{
            parser->ptr=rptr;
            parser->line=rline;
            return false;
        }
    }
    int sCount=stackPtr+1;
    while(1){
        if(stackPtr<0){
            reportError(parser,"too many assignment.");
        }
        if(!getExpression(parser,clist,envirn)){
            reportError(parser,"expected an expression in assignment.");
        }
        cmd.handle=HANDLE_POP;
        cmd.ta=DATA_REG;
        cmd.a=REG_BX;
        LIST_ADD((*clist),Cmd,cmd);
        cmd.handle=HANDLE_POPT;
        cmd.tb=DATA_INTEGER;
        cmd.a=REG_AX;
        cmd.b=stackPtr;
        LIST_ADD((*clist),Cmd,cmd);
        cmd.handle=HANDLE_SET;
        cmd.tb=DATA_REG;
        cmd.b=REG_BX;
        LIST_ADD((*clist),Cmd,cmd);
        stackPtr--;
        token=nextToken(parser);
        if(token.type==TOKEN_COMMA){
            continue;
        }else if(token.type==TOKEN_SEMI){
            while(stackPtr>=0){
                cmd.handle=HANDLE_POPT;
                cmd.tb=DATA_INTEGER;
                cmd.b=stackPtr;
                LIST_ADD((*clist),Cmd,cmd);
                cmd.handle=HANDLE_SET;
                cmd.tb=DATA_REG;
                cmd.b=REG_BX;
                LIST_ADD((*clist),Cmd,cmd);
                stackPtr--;
            }
            break;
        }else{
            reportError(parser,"expected \",\" or \";\" after the assignment.");
        }
    }
    cmd.handle=HANDLE_SFREE;
    cmd.ta=DATA_INTEGER;
    cmd.a=sCount;
    LIST_ADD((*clist),Cmd,cmd);
    return true;
}
bool getVarRef(Parser*parser,char*varName,CmdList*clist,Environment envirn){
    Cmd cmd;
    bool isFound=false;
    for(int i=0;i<parser->varlist.count;i++){
        if(strcmp(varName,parser->varlist.vals[i].name)==0){
            cmd.handle=HANDLE_MOV;
            cmd.ta=DATA_REG;
            cmd.tb=DATA_INTEGER;
            cmd.a=REG_AX;
            cmd.b=parser->varlist.vals[i].ptr;
            LIST_ADD((*clist),Cmd,cmd);
            isFound=true;
            break;
        }
    }
    if(!isFound){
        return false;
    }
    return true;
}
void getBlock(Parser*parser,CmdList*clist,VariableList*vlist,Environment envirn){
    Token token;
    int rptr,rline;
    Cmd cmd;
    token=nextToken(parser);
    if(token.type!=TOKEN_BRACE1){
        reportError(parser,"expected \"{\".");
    }
    while(1){
        rptr=parser->ptr;
        rline=parser->line;
        token=nextToken(parser);
        if(token.type==TOKEN_BRACE2){
            break;
        }else if(token.type==TOKEN_END){
            reportError(parser,"expected \"}\".");
        }
        parser->ptr=rptr;
        parser->line=rline;
        if(getVariableDef(parser,vlist,clist,envirn)){

        }else if(getAssignment(parser,clist,envirn)){

        }else if(getConditionState(parser,clist,vlist,envirn)){
            
        }else if(getWhileLoop(parser,clist,vlist,envirn)){
            
        }else if(getExpression(parser,clist,envirn)){
            token=nextToken(parser);
            if(token.type!=TOKEN_SEMI){
                reportError(parser,"expected \";\" after an expression");
            }
            cmd.handle=HANDLE_SFREE;
            cmd.ta=DATA_INTEGER;
            cmd.a=1;
            LIST_ADD((*clist),Cmd,cmd);
        }else{
            reportError(parser,"unknown expression.");
        }
    }
}
bool getConditionState(Parser*parser,CmdList*clist,VariableList*vlist,Environment envirn){
    Token token;
    int rptr,rline;
    intList ilist;
    Cmd cmd;
    int jptr;
    rptr=parser->ptr;
    rline=parser->line;
    LIST_INIT(ilist,int);
    token=nextToken(parser);
    if(token.type!=TOKEN_IF){
        parser->ptr=rptr;
        parser->line=rline;
        return false;
    }
    token=nextToken(parser);
    if(token.type!=TOKEN_PARE1){
        reportError(parser,"expected \"(\" after \"if\".");
    }
    if(!getExpression(parser,clist,envirn)){
        reportError(parser,"expected an expression in the conditional statement.");
    }
    token=nextToken(parser);
    if(token.type!=TOKEN_PARE2){
        reportError(parser,"expected \")\" after \"if(expression...\".");
    }
    cmd.handle=HANDLE_POP;
    cmd.ta=DATA_REG;
    cmd.a=REG_AX;
    LIST_ADD((*clist),Cmd,cmd);
    cmd.handle=HANDLE_MOV;
    cmd.tb=DATA_REG;
    cmd.a=REG_CF;
    cmd.b=REG_AX;
    LIST_ADD((*clist),Cmd,cmd);
    cmd.handle=HANDLE_JMPC;
    cmd.ta=DATA_INTEGER;
    cmd.a=1;
    LIST_ADD((*clist),Cmd,cmd);
    jptr=clist->count-1;
    getBlock(parser,clist,vlist,envirn);
    rptr=parser->ptr;
    rline=parser->line;
    token=nextToken(parser);
    while(token.type==TOKEN_ELIF){
        cmd.handle=HANDLE_JMP;
        cmd.ta=DATA_INTEGER;
        LIST_ADD((*clist),Cmd,cmd);
        LIST_ADD(ilist,int,clist->count-1);
        clist->vals[jptr].a=clist->count-jptr;
        if(!getExpression(parser,clist,envirn)){
            reportError(parser,"expected an expression in the conditional statement.");
        }
        cmd.handle=HANDLE_POP;
        cmd.ta=DATA_REG;
        cmd.a=REG_AX;
        LIST_ADD((*clist),Cmd,cmd);
        cmd.handle=HANDLE_MOV;
        cmd.tb=DATA_REG;
        cmd.a=REG_CF;
        cmd.b=REG_AX;
        LIST_ADD((*clist),Cmd,cmd);
        cmd.handle=HANDLE_JMPC;
        cmd.ta=DATA_INTEGER;
        cmd.a=1;
        LIST_ADD((*clist),Cmd,cmd);
        jptr=clist->count-1;
        getBlock(parser,clist,vlist,envirn);
        rptr=parser->ptr;
        rline=parser->line;
        token=nextToken(parser);
    }
    clist->vals[jptr].a=clist->count-jptr;
    if(token.type==TOKEN_ELSE){
        cmd.handle=HANDLE_JMP;
        cmd.ta=DATA_INTEGER;
        LIST_ADD((*clist),Cmd,cmd);
        LIST_ADD(ilist,int,clist->count-1);
        getBlock(parser,clist,vlist,envirn);
    }else{
        parser->ptr=rptr;
        parser->line=rline;
    }
    for(int i=0;i<ilist.count;i++){
        clist->vals[ilist.vals[i]].a=clist->count-ilist.vals[i];
    }
    LIST_DELETE(ilist)
    return true;
}
bool getWhileLoop(Parser*parser,CmdList*clist,VariableList*vlist,Environment envirn){
    Token token;
    Cmd cmd;
    int rptr,rline;
    int jptr,wptr;
    rptr=parser->ptr;
    rline=parser->line;
    token=nextToken(parser);
    if(token.type!=TOKEN_WHILE){
        parser->ptr=rptr;
        parser->line=rline;
        return false;
    }
    token=nextToken(parser);
    if(token.type!=TOKEN_PARE1){
        reportError(parser,"expected \"(\" after \"while\".");
    }
    wptr=clist->count;
    if(!getExpression(parser,clist,envirn)){
        reportError(parser,"expected an expression in the loop statement.");
    }
    token=nextToken(parser);
    if(token.type!=TOKEN_PARE2){
        reportError(parser,"expected \")\" after \"while\".");
    }
    cmd.handle=HANDLE_POP;
    cmd.ta=DATA_REG;
    cmd.a=REG_AX;
    LIST_ADD((*clist),Cmd,cmd);
    cmd.handle=HANDLE_MOV;
    cmd.tb=DATA_REG;
    cmd.a=REG_CF;
    cmd.b=REG_AX;
    LIST_ADD((*clist),Cmd,cmd);
    cmd.handle=HANDLE_JMPC;
    cmd.ta=DATA_INTEGER;
    LIST_ADD((*clist),Cmd,cmd);
    jptr=clist->count-1;
    getBlock(parser,clist,vlist,envirn);
    cmd.handle=HANDLE_JMP;
    cmd.ta=DATA_INTEGER;
    cmd.a=-(clist->count-wptr);
    LIST_ADD((*clist),Cmd,cmd);
    clist->vals[jptr].a=clist->count-jptr;
    return true;
}