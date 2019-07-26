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
        if(strcmp(token.word,"int")==0){
            token.type=TOKEN_INT;
        }else if(strcmp(token.word,"float")==0){
            token.type=TOKEN_FLOAT_CLASS;
        }else if(strcmp(token.word,"func")==0){
            token.type=TOKEN_FUNC;
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
        }else if(strcmp(token.word,"break")==0){
            token.type=TOKEN_BREAK;
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
HandleType handleFloat(HandleType ht){
    switch (ht)
    {
    case HANDLE_ADD:
        return HANDLE_FADD;
        break;
    case HANDLE_SUB:
        return HANDLE_FSUB;
        break;
    case HANDLE_MUL:
        return HANDLE_FMUL;
        break;
    case HANDLE_DIV:
        return HANDLE_FDIV;
        break;
    default:
        return ht;
        break;
    }
    return ht;
}
bool getExpression(Parser*parser,CmdList*clist,int*rclass,Environment envirn){
    Token token;
    OperatList olist;
    Operat operat;
    char msg[100];
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
        operat.isVar=false;
        if(token.type==TOKEN_PARE1){
            if(!getExpression(parser,clist,&operat.class,envirn)){
                reportWarning(parser,"expected an expression in the \"( )\".");
            }
            token=nextToken(parser);
            if(token.type!=TOKEN_PARE2){
                reportError(parser,"expected \")\".");
            }
        }else if(token.type==TOKEN_INTEGER || token.type==TOKEN_FLOAT){
            addCmd1(clist,HANDLE_PUSH,DATA_INTEGER,token.num);
            operat.class=(token.type==TOKEN_INTEGER)?TYPE_INTEGER:TYPE_FLOAT;
        }else if(token.type==TOKEN_WORD){
            if(!getVarRef(parser,token.word,clist,&operat.class,envirn)){
                sprintf(msg,"unfound variable \"%s\".",token.word);
                reportError(parser,msg);
            }
            addCmd1(clist,HANDLE_PUSH,DATA_REG,REG_AX);
            operat.isVar=true;
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
        bool isRight=false;
        if(olist.count>=1){
            if(olist.vals[olist.count-1].power>=operat.power){
                isRight=true;
            }
        }
        if(isEnd){
            isRight=true;
        }
        if(isRight){
            addCmd1(clist,HANDLE_POP,DATA_REG,REG_BX);
            if(operat.handle_postfix==HANDLE_ADD){
                if(operat.isVar){
                    if(operat.class==TYPE_INTEGER){
                        addCmd2(clist,HANDLE_ADD,DATA_REG_POINTER,DATA_INTEGER,REG_BX,1);
                    }else if(operat.class==TYPE_FLOAT){
                        addCmd2(clist,HANDLE_FADD,DATA_REG_POINTER,DATA_INTEGER,REG_BX,1);
                    }else{
                        sprintf(msg,"the type \"%s\" does not support postfix calculation.",parser->classList.vals[operat.class].name);
                        reportWarning(parser,msg);
                    }
                }else{
                    if(operat.class==TYPE_INTEGER){
                        addCmd2(clist,HANDLE_ADD,DATA_REG,DATA_INTEGER,REG_BX,1);
                    }else if(operat.class==TYPE_FLOAT){
                        addCmd2(clist,HANDLE_FADD,DATA_REG,DATA_INTEGER,REG_BX,1);
                    }else{
                            reportError(parser,"unknown constant.");
                    }
                }
                operat.handle_postfix=HANDLE_NOP;
            }
            if(operat.isVar){
                addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG_POINTER,REG_BX,REG_BX);
                operat.isVar=false;
            }
            if(operat.handle_prefix!=HANDLE_NOP){
                addCmd1(clist,operat.handle_prefix,DATA_REG,REG_BX);
                operat.handle_prefix=HANDLE_NOP;
            }
            while(olist.count>=1){
                Operat opt=olist.vals[olist.count-1];
                if(operat.power>opt.power && !isEnd){
                    break;
                }
                addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
                if(opt.handle_postfix==HANDLE_ADD){
                    if(opt.isVar){
                        if(opt.class==TYPE_INTEGER){
                            addCmd2(clist,HANDLE_ADD,DATA_REG_POINTER,DATA_INTEGER,REG_AX,1);
                        }else if(operat.class==TYPE_FLOAT){
                            addCmd2(clist,HANDLE_FADD,DATA_REG_POINTER,DATA_INTEGER,REG_AX,1);
                        }else{
                            sprintf(msg,"the type \"%s\" does not support postfix calculation.",parser->classList.vals[operat.class].name);
                            reportWarning(parser,msg);
                        }
                    }else{
                        if(opt.class==TYPE_INTEGER){
                            addCmd2(clist,HANDLE_ADD,DATA_REG,DATA_INTEGER,REG_AX,1);
                        }else if(opt.class==TYPE_FLOAT){
                            addCmd2(clist,HANDLE_FADD,DATA_REG,DATA_INTEGER,REG_AX,1);
                        }else{
                            reportError(parser,"unknown constant.");
                        }
                    }
                    opt.handle_postfix=HANDLE_NOP;
                }
                if(opt.isVar){
                    addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG_POINTER,REG_AX,REG_AX);
                }
                if(opt.handle_prefix!=HANDLE_NOP){
                    addCmd1(clist,opt.handle_prefix,DATA_REG,REG_AX);
                }
                if(opt.class==TYPE_FLOAT){
                    if(operat.class!=TYPE_FLOAT && operat.class!=TYPE_INTEGER){
                        sprintf(msg,"can not cover type \"%s\" to \"%s\".",parser->classList.vals[TYPE_FLOAT].name,parser->classList.vals[operat.class].name);
                        reportError(parser,msg);
                    }
                    opt.handle_infix=handleFloat(opt.handle_infix);
                }else if(opt.class==TYPE_INTEGER){
                    if(operat.class==TYPE_FLOAT){
                        opt.handle_infix=handleFloat(opt.handle_infix);
                    }else if(operat.class!=TYPE_INTEGER){
                        sprintf(msg,"can not cover type \"%s\" to \"%s\".",parser->classList.vals[TYPE_INTEGER].name,parser->classList.vals[operat.class].name);
                        reportError(parser,msg);
                    }
                }
                addCmd2(clist,opt.handle_infix,DATA_REG,DATA_REG,REG_AX,REG_BX);
                addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG,REG_BX,REG_AX);
                operat.class=opt.class;
                LIST_SUB(olist,Operat);
            }
            addCmd1(clist,HANDLE_PUSH,DATA_REG,REG_BX);
        }
        if(isEnd){
            *rclass=operat.class;
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
    char msg[100];
    int rptr=parser->ptr;
    int rline=parser->line;
    int rclass;
    token=nextToken(parser);
    if(token.type==TOKEN_INT){
        var.class=TYPE_INTEGER;
    }else if(token.type==TOKEN_FLOAT_CLASS){
        var.class=TYPE_FLOAT;
    }else if(token.type==TOKEN_WORD){
        bool isFound=false;
        for(int i=2;i<parser->classList.count;i++){
            if(strcmp(token.word,parser->classList.vals[i].name)==0){
                var.class=i;
                isFound=true;
            }
        }
        if(!isFound){
            parser->ptr=rptr;
            parser->line=rline;
            return false;  
        }
    }else{
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
        var.ptr=parser->dataSize;
        parser->dataSize+=parser->classList.vals[var.class].size;
        LIST_ADD((*vlist),Variable,var);
        token=nextToken(parser);
        if(token.type==TOKEN_EQUAL){
            if(!getExpression(parser,clist,&rclass,envirn)){
                reportError(parser,"expected an expression when initializing variable");
            }
            if((var.class==TYPE_FLOAT || var.class==TYPE_INTEGER) && (rclass==TYPE_FLOAT || rclass==TYPE_INTEGER)){
                addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
                addCmd2(clist,HANDLE_MOV,DATA_POINTER,DATA_REG,var.ptr,REG_AX);
            }else if(var.class!=rclass){
                sprintf(msg,"incompatible types when assigning to type \"%s\" from type \"%s\".",parser->classList.vals[var.class].name,parser->classList.vals[rclass].name);
                reportError(parser,msg);
            }else{
                /*for(int i=0;i<parser->classList.vals[rclass].size;i++){
                    addCmd2(clist,HANDLE_MOV,DATA_POINTER,DATA_POINTER,var.ptr+i,)
                }*/
                addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
                addCmd2(clist,HANDLE_MOV,DATA_POINTER,DATA_REG,var.ptr,REG_AX);
            }
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
    int class;
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
        if(!getVarRef(parser,token.word,clist,&class,envirn)){
            parser->ptr=rptr;
            parser->line=rline;
            return false;
        }
        addCmd1(clist,HANDLE_PUSH,DATA_REG,REG_AX);
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
    int rclass;
    while(1){
        if(stackPtr<0){
            reportError(parser,"too many assignment.");
        }
        if(!getExpression(parser,clist,&rclass,envirn)){
            reportError(parser,"expected an expression in assignment.");
        }
        addCmd1(clist,HANDLE_POP,DATA_REG,REG_BX);
        addCmd2(clist,HANDLE_POPT,DATA_REG,DATA_INTEGER,REG_AX,stackPtr);
        addCmd2(clist,HANDLE_MOV,DATA_REG_POINTER,DATA_REG,REG_AX,REG_BX);
        stackPtr--;
        token=nextToken(parser);
        if(token.type==TOKEN_COMMA){
            continue;
        }else if(token.type==TOKEN_SEMI){
            while(stackPtr>=0){
                addCmd2(clist,HANDLE_POPT,DATA_REG,DATA_INTEGER,REG_AX,stackPtr);
                addCmd2(clist,HANDLE_MOV,DATA_REG_POINTER,DATA_REG,REG_AX,REG_BX);
                stackPtr--;
            }
            break;
        }else{
            reportError(parser,"expected \",\" or \";\" after the assignment.");
        }
    }
    addCmd1(clist,HANDLE_SFREE,DATA_INTEGER,sCount);
    return true;
}
bool getVarRef(Parser*parser,char*varName,CmdList*clist,int*class,Environment envirn){
    bool isFound=false;
    for(int i=0;i<parser->varlist.count;i++){
        if(strcmp(varName,parser->varlist.vals[i].name)==0){
            *class=parser->varlist.vals[i].class;
            addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_INTEGER,REG_AX,parser->varlist.vals[i].ptr);
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
    int rclass;
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
        }else if(token.type==TOKEN_BREAK){
            token=nextToken(parser);
            if(token.type!=TOKEN_SEMI){
                reportError(parser,"expected \";\" after \"break\".");
            }
            if(envirn.breakList==NULL){
                reportWarning(parser,"unuseful break.");
            }else{
                addCmd1(clist,HANDLE_JMP,DATA_INTEGER,1);
                LIST_ADD((*envirn.breakList),int,clist->count-1);
            }
            continue;
        }else if(token.type==TOKEN_END){
            reportError(parser,"expected \"}\".");
        }
        parser->ptr=rptr;
        parser->line=rline;
        if(getVariableDef(parser,vlist,clist,envirn)){

        }else if(getAssignment(parser,clist,envirn)){

        }else if(getConditionState(parser,clist,vlist,envirn)){
            
        }else if(getWhileLoop(parser,clist,vlist,envirn)){
            
        }else if(getExpression(parser,clist,&rclass,envirn)){
            token=nextToken(parser);
            if(token.type!=TOKEN_SEMI){
                reportError(parser,"expected \";\" after an expression");
            }
            addCmd1(clist,HANDLE_SFREE,DATA_INTEGER,1);
        }else{
            reportError(parser,"unknown expression.");
        }
    }
}
bool getConditionState(Parser*parser,CmdList*clist,VariableList*vlist,Environment envirn){
    Token token;
    int rptr,rline;
    intList ilist;
    int jptr;
    int rclass;
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
    if(!getExpression(parser,clist,&rclass,envirn)){
        reportError(parser,"expected an expression in the conditional statement.");
    }
    token=nextToken(parser);
    if(token.type!=TOKEN_PARE2){
        reportError(parser,"expected \")\" after \"if(expression...\".");
    }
    addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
    addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG,REG_CF,REG_AX);
    addCmd1(clist,HANDLE_JMPC,DATA_INTEGER,1);
    jptr=clist->count-1;
    getBlock(parser,clist,vlist,envirn);
    rptr=parser->ptr;
    rline=parser->line;
    token=nextToken(parser);
    while(token.type==TOKEN_ELIF){
        addCmd1(clist,HANDLE_JMP,DATA_INTEGER,1);
        LIST_ADD(ilist,int,clist->count-1);
        clist->vals[jptr].a=clist->count-jptr;
        token=nextToken(parser);
        if(token.type!=TOKEN_PARE1){
            reportError(parser,"expected \"(\" after \"if(expression...\".");
        }
        if(!getExpression(parser,clist,&rclass,envirn)){
            reportError(parser,"expected an expression in the conditional statement.");
        }
        token=nextToken(parser);
        if(token.type!=TOKEN_PARE2){
            reportError(parser,"expected \")\" after \"if\".");
        }
        addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
        addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG,REG_CF,REG_AX);
        addCmd1(clist,HANDLE_JMPC,DATA_INTEGER,1);
        jptr=clist->count-1;
        getBlock(parser,clist,vlist,envirn);
        rptr=parser->ptr;
        rline=parser->line;
        token=nextToken(parser);
    }
    if(token.type==TOKEN_ELSE){
        addCmd1(clist,HANDLE_JMP,DATA_INTEGER,1);
        LIST_ADD(ilist,int,clist->count-1);
        getBlock(parser,clist,vlist,envirn);
    }else{
        parser->ptr=rptr;
        parser->line=rline;
    }
    clist->vals[jptr].a=clist->count-jptr;
    for(int i=0;i<ilist.count;i++){
        clist->vals[ilist.vals[i]].a=clist->count-ilist.vals[i];
    }
    LIST_DELETE(ilist)
    return true;
}
bool getWhileLoop(Parser*parser,CmdList*clist,VariableList*vlist,Environment envirn){
    Token token;
    int rptr,rline;
    int jptr,wptr;
    int rclass;
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
    if(!getExpression(parser,clist,&rclass,envirn)){
        reportError(parser,"expected an expression in the loop statement.");
    }
    token=nextToken(parser);
    if(token.type!=TOKEN_PARE2){
        reportError(parser,"expected \")\" after \"while\".");
    }
    intList breakList;
    LIST_INIT(breakList,int);
    envirn.breakList=&breakList;
    addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
    addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG,REG_CF,REG_AX);
    addCmd1(clist,HANDLE_JMPC,DATA_INTEGER,1);
    jptr=clist->count-1;
    rptr=parser->ptr;
    rline=parser->line;
    token=nextToken(parser);
    if(token.type!=TOKEN_SEMI){
        parser->line=rline;
        parser->ptr=rptr;
        getBlock(parser,clist,vlist,envirn);
    }
    addCmd1(clist,HANDLE_JMP,DATA_INTEGER,-(clist->count-wptr));
    clist->vals[jptr].a=clist->count-jptr;
    for(int i=0;i<breakList.count;i++){
        clist->vals[breakList.vals[i]].a=clist->count-breakList.vals[i];
    }
    LIST_DELETE(breakList);
    return true;
}