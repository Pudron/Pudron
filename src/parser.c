#include"parser.h"
const int SYMBOL_COUNT=35;
const int KEYWORD_COUNT=12;
const TokenSymbol symbolList[]={
    /*多字符运算符放前面*/
    {TOKEN_LEFT_EQUAL,"<<=",3},
    {TOKEN_RIGHT_EQUAL,">>=",3},
    //{TOKEN_DOUBLE_ADD,"++",2},
    //{TOKEN_DOUBLE_SUB,"--",2},
    {TOKEN_GTHAN_EQUAL,">=",2},
    {TOKEN_LTHAN_EQUAL,"<=",2},
    {TOKEN_NOT_EQUAL,"!=",2},
    {TOKEN_LEFT,"<<",2},
    {TOKEN_RIGHT,">>",2},
    {TOKEN_ADD_EQUAL,"+=",2},
    {TOKEN_SUB_EQUAL,"-=",2},
    {TOKEN_MUL_EQUAL,"*=",2},
    {TOKEN_DIV_EQUAL,"/=",2},
    {TOKEN_AND_EQUAL,"&=",2},
    {TOKEN_OR_EQUAL,"|=",2},
    {TOKEN_PERCENT_EQUAL,"%=",2},
    {TOKEN_ADD,"+",1},
    {TOKEN_SUB,"-",1},
    {TOKEN_MUL,"*",1},
    {TOKEN_DIV,"/",1},
    {TOKEN_EXCL,"!",1},
    {TOKEN_EQUAL,"=",1},
    {TOKEN_POINT,".",1},
    {TOKEN_COMMA,",",1},
    {TOKEN_SEMI,";",1},
    {TOKEN_PARE1,"(",1},
    {TOKEN_PARE2,")",1},
    {TOKEN_BRACE1,"{",1},
    {TOKEN_BRACE2,"}",1},
    {TOKEN_BRACKET1,"[",1},
    {TOKEN_BRACKET2,"]",1},
    {TOKEN_GTHAN,">",1},
    {TOKEN_LTHAN,"<",1},
    {TOKEN_INVERT,"~",1},
    {TOKEN_AND,"&",1},
    {TOKEN_OR,"|",1},
    {TOKEN_PERCENT,"%",1}
};
const Keyword keywordList[]={
    {TOKEN_FUNC,"func","函数"},
    {TOKEN_WHILE,"while","当"},
    {TOKEN_BREAK,"break","结束"},
    {TOKEN_RETURN,"return","返回"},
    {TOKEN_IF,"if","如果"},
    {TOKEN_ELIF,"elif","若"},
    {TOKEN_ELSE,"else","否则"},
    {TOKEN_CAND,"and","且"},
    {TOKEN_COR,"or","或"},
    {TOKEN_NULL,"null","无"},
    {TOKEN_CAND,"and","且"},
    {TOKEN_COR,"or","或"}
};
const int OPERAT_PREFIX_COUNT=3;
const Operat operatPrefix[]={
    {TOKEN_EXCL,OPCODE_NOT,130},
    {TOKEN_INVERT,OPCODE_INVERT,180},
    {TOKEN_SUB,OPCODE_SUBS,100}
};
const int OPERAT_INFIX_COUNT=15;
const Operat operatInfix[]={
    {TOKEN_ADD,OPCODE_ADD,100},
    {TOKEN_SUB,OPCODE_SUB,100},
    {TOKEN_MUL,OPCODE_MUL,120},
    {TOKEN_DIV,OPCODE_DIV,120},
    {TOKEN_AND,OPCODE_AND,150},
    {TOKEN_OR,OPCODE_OR,150},
    {TOKEN_EQUAL,OPCODE_EQUAL,50},
    {TOKEN_NOT_EQUAL,OPCODE_NOT_EQUAL,50},
    {TOKEN_PERCENT,OPCODE_REM,120},
    {TOKEN_LEFT,OPCODE_LEFT,160},
    {TOKEN_RIGHT,OPCODE_RIGHT,160},
    {TOKEN_GTHAN,OPCODE_GTHAN,50},
    {TOKEN_LTHAN,OPCODE_LTHAN,50},
    {TOKEN_GTHAN_EQUAL,OPCODE_GTHAN_EQUAL,50},
    {TOKEN_LTHAN_EQUAL,OPCODE_LTHAN_EQUAL,50}
};
/*Token*/
Token nextToken(Parser*parser){
    char word[1024];
    Msg msg;
    Token token;
    bool isFound;
    char c=parser->code[parser->ptr];
    while(c==' ' || c=='	' || c=='\n' || c=='#'){
        parser->column++;
        if(c=='\n'){
            parser->line++;
            parser->column=1;
        }
        if(c=='#'){
            /*跳过注释*/
            msg.column=parser->column;
            msg.line=parser->line;
            msg.start=parser->ptr;
            c=parser->code[++parser->ptr];
            while(c!='#'){
                if(c=='\0'){
                    msg.type=MSG_ERROR;
                    if(parser->ptr-msg.start<20){
                        msg.end=parser->ptr;
                    }else{
                        msg.end=msg.start+20;
                    }
                    strcpy(msg.text,"unterminated comment.");
                    reportMsg(parser,msg);
                }
                parser->column++;
                if(c=='\n'){
                    parser->line++;
                    parser->column=1;
                }
                c=parser->code[++parser->ptr];
            }
        }
        c=parser->code[++parser->ptr];
    }
    msg.start=parser->ptr;
    if(c=='\0'){
        token.type=TOKEN_END;
        return token;
    }
    if(c>='0' && c<='9'){
        int num=0,dat=0;
        msg.start=parser->ptr;
        while(c>='0' && c<='9'){
            num=num*10+c-'0';
            parser->column++;
            c=parser->code[++parser->ptr];
        }
        token.type=TOKEN_INTEGER;
        token.num=num;
        if(c=='.'){
            c=parser->code[++parser->ptr];
            while(c>='0' && c<='9'){
                dat++;
                num=num*10+c-'0';
                parser->column++;
                c=parser->code[++parser->ptr];
            }
            if(dat==0){
                parser->ptr--;
            }else{
                token.type=TOKEN_FLOAT;
                token.numf=num;
                for(int i=0;i<dat;i++){
                    token.numf/=10;
                }
            }
        }
        return token;
    }else if((c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_' || (c>='0' && c<='9')){
        int i=0;
        msg.line=parser->line;
        msg.column=parser->column;
        msg.start=parser->ptr;
        for(i=0;(c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_' || (c>='0' && c<='9');i++){
            if(i>=1023){
                msg.type=MSG_ERROR;
                msg.end=msg.start+99;
                strcpy(msg.text,"too long word.");
                reportMsg(parser,msg);
            }
            word[i]=c;
            parser->column++;
            c=parser->code[++parser->ptr];
        }
        word[i]='\0';
        parser->column+=i;
        token.word=(char*)malloc(i+1);
        strcpy(token.word,word);
        token.type=TOKEN_WORD;
    }else if(c=='\''){
        int i=0;
        msg.line=parser->line;
        msg.column=parser->column;
        msg.start=parser->ptr;
        msg.type=MSG_ERROR;
        c=parser->code[++parser->ptr];
        for(i=0;c!='\'';i++){
            if(c=='\0'){
                msg.end=parser->ptr;
                strcpy(msg.text,"expected \' after the name.");
                reportMsg(parser,msg);
            }
            if(i>=1023){
                msg.end=parser->ptr;
                strcpy(msg.text,"the name is too long.");
                reportMsg(parser,msg);
            }
            if(c=='\n'){
                parser->line++;
                parser->column=1;
                i--;
                c=parser->code[++parser->ptr];
                continue;
            }
            word[i]=c;
            c=parser->code[++parser->ptr];
            parser->column++;
        }
        word[i]='\0';
        token.word=(char*)malloc(i+1);
        strcpy(token.word,word);
        token.type=TOKEN_WORD;
    }else{
        TokenSymbol symbol;
        for(int i=0;i<SYMBOL_COUNT;i++){
            symbol=symbolList[i];
            isFound=false;
            for(int i2=0;i2<symbol.len;i2++){
                if(symbol.text[i2]==parser->code[parser->ptr+i2]){
                    isFound=true;
                }else{
                    isFound=false;
                    break;
                }
            }
            if(isFound){
                token.type=symbol.type;
                parser->ptr+=symbol.len;
                parser->column+=symbol.len;
                return token;
            }
        }
        sprintf(msg.text,"unknown charactor \"%c\".",c);
        reportError(parser,msg.text,msg.start);
    }
    /*查找关键字*/
    if(token.type==TOKEN_WORD){
        for(int i=0;i<KEYWORD_COUNT;i++){
            if(strcmp(token.word,keywordList[i].name)==0 || strcmp(token.word,keywordList[i].other)==0){
                token.type=keywordList[i].type;
                free(token.word);
                token.word='\0';
                return token;
            }
        }
    }
    return token;
}
Token matchToken(Parser*parser,TokenType et,char*str,int start){
    Token token=nextToken(parser);
    Msg msg;
    if(token.type!=et){
        msg.type=MSG_ERROR;
        msg.line=parser->line;
        msg.column=parser->column;
        msg.start=start;
        msg.end=parser->ptr;
        sprintf(msg.text,"expect %s.",str);
        reportMsg(parser,msg);
    }
    return token;
}
/*Token end*/
bool getValue(Parser*parser,intList*clist,Command*gcmds,Command*asCmd,Env env){
    Token token;
    Symbol symbol;
    Command cmds,acmds;
    bool canAssign;
    char msgText[50];
    ORI_DEF()
    ORI_ASI()
    int msgStart=parser->ptr;
    token=nextToken(parser);
    if(token.type==TOKEN_INTEGER){
        symbol.type=SYM_INT;
        symbol.num=token.num;
        addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
        canAssign=false;
    }else if(token.type==TOKEN_FLOAT){
        symbol.type=SYM_FLOAT;
        symbol.numf=token.numf;
        addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
        canAssign=false;
    }else if(token.type==TOKEN_PARE1){
        getExpression(parser,clist,0,env);
        matchToken(parser,TOKEN_PARE2,"\")\"",msgStart);
        canAssign=false;
    }else if(token.type==TOKEN_WORD){
        bool isFound=false;
        if(env.classDef!=NULL){
            for(int i=0;i<env.classDef->var.count;i++){
                if(strcmp(token.word,env.classDef->var.vals[i])==0){
                    if(env.isClassVarDef){
                        sprintf(msgText,"the attr \"%s\" in the class \"%s\" has already existed.",token.word,env.classDef->name);
                        reportError(parser,msgText,msgStart);
                    }
                    symbol.type=SYM_STRING;
                    symbol.str=(char*)malloc(5);
                    strcpy(symbol.str,"this");
                    addCmd1(parser,clist,OPCODE_LOAD_VAL,addSymbol(parser,symbol));
                    isFound=true;
                    break;
                }
            }
        }
        if(!isFound && env.isClassVarDef){
            LIST_ADD(env.classDef->var,Name,token.word)
            symbol.type=SYM_STRING;
            symbol.str=(char*)malloc(5);
            strcpy(symbol.str,"this");
            addCmd1(parser,clist,OPCODE_LOAD_VAL,addSymbol(parser,symbol));
            isFound=true;
        }
        symbol.type=SYM_STRING;
        symbol.str=token.word;
        cmds.code[0]=isFound?OPCODE_LOAD_ATTR:OPCODE_LOAD_VAL;
        cmds.code[1]=addSymbol(parser,symbol);
        cmds.count=2;
        acmds.code[0]=isFound?OPCODE_STORE_ATTR:OPCODE_STORE_VAL;
        acmds.code[1]=cmds.code[1];
        acmds.count=2;
        canAssign=true;
    }else{
        ORI_RET()
        return false;
    }
    while(1){
        ORI_ASI()
        token=nextToken(parser);
        if(token.type==TOKEN_POINT){
            if(env.isClassVarDef){
                reportError(parser,"uninitialized attr",msgStart);
            }
            if(canAssign){
                addCmds(parser,clist,cmds);
            }
            token=nextToken(parser);
            if(token.type!=TOKEN_WORD){
                reportError(parser,"expect an attr in expression.",msgStart);
            }
            symbol.type=SYM_STRING;
            symbol.str=token.word;
            cmds.code[0]=OPCODE_LOAD_ATTR;
            cmds.code[1]=addSymbol(parser,symbol);
            cmds.count=2;
            acmds.code[0]=OPCODE_STORE_ATTR;
            acmds.code[1]=cmds.code[1];
            acmds.count=2;
            canAssign=true;
        }else if(token.type==TOKEN_BRACKET1){
            if(env.isClassVarDef){
                reportError(parser,"uninitialized attr",msgStart);
            }
            if(canAssign){
                addCmds(parser,clist,cmds);
            }
            canAssign=true;
            getExpression(parser,clist,0,env);
            matchToken(parser,TOKEN_BRACKET2,"\"]\"",msgStart);
            cmds.code[0]=OPCODE_LOAD_INDEX;
            cmds.count=1;
            acmds.code[0]=OPCODE_STORE_INDEX;
            acmds.count=1;
        }/*else if(token.type==TOKEN_DOUBLE_ADD){
            if(canAssign){
                if(cmds.code[0]==OPCODE_LOAD_INDEX){
                    addCmd1(parser,clist,OPCODE_STACK_COPY,1);
                    addCmd1(parser,clist,OPCODE_STACK_COPY,1);
                    addCmds(parser,clist,cmds);
                }else{
                    addCmd1(parser,clist,OPCODE_STACK_COPY,0);
                    addCmds(parser,clist,cmds);
                }
            }
            symbol.type=SYM_INT;
            symbol.num=1;
            addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
            addCmd(parser,clist,OPCODE_ADD);
            if(canAssign){
                if(cmds.code[0]==OPCODE_LOAD_INDEX){
                    addCmd1(parser,clist,OPCODE_STACK_COPY,2);
                    addCmd1(parser,clist,OPCODE_STACK_COPY,2);
                    addCmds(parser,clist,acmds);
                }else{
                    addCmd1(parser,clist,OPCODE_STACK_COPY,1);
                    addCmds(parser,clist,acmds);
                }
            }
            break;
        }*/else{
            ORI_RET()
            break;
        }
    }
    if(!canAssign && asCmd!=NULL){
        reportError(parser,"assignment unsupported.",msgStart);
    }
    if(canAssign && asCmd!=NULL){
        *asCmd=acmds;
        if(gcmds!=NULL){
            *gcmds=cmds;
        }
    }else if(canAssign){
        addCmds(parser,clist,cmds);
    }
    return true;
}
Operat getExpression(Parser*parser,intList*clist,int level,Env env){
    Token token;
    Operat opt;
    Opcode opcode;
    ORI_DEF()
    bool isFound=false;
    env.isClassVarDef=false;
    bool isVal=getValue(parser,clist,NULL,NULL,env);
    int msgStart=parser->ptr;
    ORI_ASI()
    token=nextToken(parser);
    if(isVal){
        for(int i=0;i<OPERAT_INFIX_COUNT;i++){
            if(token.type==operatInfix[i].tokenType){
                opt=operatInfix[i];
                isFound=true;
            }
        }
        if(!isFound){
            opt.opcode=OPCODE_NOP;
            opt.level=0;
            ORI_RET()
        }
    }else{
        for(int i=0;i<OPERAT_PREFIX_COUNT;i++){
            if(token.type==operatPrefix[i].tokenType){
                opt=operatPrefix[i];
                isFound=true;
            }
        }
        if(!isFound){
            reportError(parser,"expected an expression.",msgStart);
        }
        if(level){
            reportWarning(parser,"prefix operator used be in front of the expression.",msgStart);
        }
    }
    while(level<opt.level){
        opcode=opt.opcode;
        opt=getExpression(parser,clist,opt.level,env);
        addCmd(parser,clist,opcode);
    }
    return opt;
}
bool getAssignment(Parser*parser,intList*clist,Env env){
    Token token;
    ORI_DEF()
    ORI_ASI()
    AssignList assignList;
    Assign assign;
    Symbol symbol;
    int opcode=OPCODE_NOP;
    bool isAssign=false;/*确定是否是赋值表达式*/
    int msgStart=parser->ptr;
    LIST_INIT(assignList,Assign)
    while(1){
        LIST_INIT(assign.clist,int)
        if(!getValue(parser,&assign.clist,&assign.gcmds,&assign.acmds,env)){
            if(!isAssign){
                ORI_RET()
                LIST_DELETE(assign.clist)
                LIST_DELETE(assignList)
                return false;
            }else{
                reportError(parser,"expected a variable to assign.",msgStart);
            }
        }
        LIST_ADD(assignList,Assign,assign)
        token=nextToken(parser);
        if(token.type!=TOKEN_COMMA){
            break;
        }
        isAssign=true;
    }
    if(token.type==TOKEN_SEMI){
        /*全部创建或初始化变量*/
        for(int i=0;i<assignList.count;i++){
            assign=assignList.vals[i];
            symbol.type=SYM_INT;
            symbol.num=0;
            addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
            LIST_CONNECT((*clist),assign.clist,int)
            LIST_DELETE(assign.clist)
            addCmds(parser,clist,assign.acmds);
        }
        return true;
    }else if(token.type==TOKEN_EQUAL){
        opcode=OPCODE_NOP;
    }else if(token.type==TOKEN_ADD_EQUAL){
        opcode=OPCODE_ADD;
    }else if(token.type==TOKEN_SUB_EQUAL){
        opcode=OPCODE_SUB;
    }else if(token.type==TOKEN_AND_EQUAL){
        opcode=OPCODE_AND;
    }else if(token.type==TOKEN_OR_EQUAL){
        opcode=OPCODE_OR;
    }else if(token.type==TOKEN_PERCENT_EQUAL){
        opcode=OPCODE_REM;
    }else if(token.type==TOKEN_LEFT_EQUAL){
        opcode=OPCODE_LEFT;
    }else if(token.type==TOKEN_RIGHT_EQUAL){
        opcode=OPCODE_RIGHT;
    }else{
        if(isAssign){
            reportError(parser,"expected \",\" or \";\" or assignment.",msgStart);
        }
        LIST_DELETE(assign.clist)
        LIST_DELETE(assignList)
        ORI_RET()
        return false;
    }
    bool isEnd=false;
    for(int i=0;i<assignList.count;i++){
        assign=assignList.vals[i];
        if(isEnd && i!=assignList.count-1){
            addCmd1(parser,clist,OPCODE_STACK_COPY,0);
        }else if(!isEnd){
            getExpression(parser,clist,0,env);
        }
        if(!isEnd){
            token=nextToken(parser);
            if(token.type==TOKEN_SEMI){
                isEnd=true;
                if(i!=assignList.count-1){
                    addCmd1(parser,clist,OPCODE_STACK_COPY,0);
                }
            }else if(token.type!=TOKEN_COMMA){
                reportError(parser,"expected \",\" or \";\" in assignment.",msgStart);
            }
        }
        LIST_CONNECT((*clist),assign.clist,int)
        LIST_DELETE(assign.clist);
        if(opcode!=OPCODE_NOP){
            if(assign.acmds.code[0]==OPCODE_STORE_INDEX){
                addCmd1(parser,clist,OPCODE_STACK_COPY,1);
                addCmd1(parser,clist,OPCODE_STACK_COPY,1);
            }else{
                addCmd1(parser,clist,OPCODE_STACK_COPY,0);
            }
            addCmds(parser,clist,assign.gcmds);
            addCmd1(parser,clist,OPCODE_STACK_COPY,1);
            addCmd(parser,clist,opcode);
            if(assign.acmds.code[0]==OPCODE_STORE_INDEX){
                addCmd1(parser,clist,OPCODE_STACK_COPY,2);
                addCmd1(parser,clist,OPCODE_STACK_COPY,2);
                addCmds(parser,clist,assign.acmds);
                addCmd1(parser,clist,OPCODE_POP_STACK,3);
            }else{
                addCmd1(parser,clist,OPCODE_STACK_COPY,1);
                addCmds(parser,clist,assign.acmds);
                addCmd1(parser,clist,OPCODE_POP_STACK,2);
            }
        }else{
            addCmds(parser,clist,assign.acmds);
        }
    }
    if(token.type==TOKEN_COMMA){
        reportError(parser,"too many assignments.",msgStart);
    }
    LIST_DELETE(assignList)
    return true;
}
void getBlock(Parser*parser,intList*clist,Env env){
    Token token;
    int msgStart=parser->ptr;
    PartMsg part;
    ORI_DEF()
    matchToken(parser,TOKEN_BRACE1,"\"{\"",msgStart);
    addCmd(parser,clist,OPCODE_SET_VALS);
    while(1){
        part.line=parser->line;
        part.column=parser->column;
        part.start=parser->ptr;
        msgStart=parser->ptr;
        ORI_ASI()
        token=nextToken(parser);
        if(token.type==TOKEN_END){
            reportError(parser,"expected \"}\" after a block.",msgStart);
        }else if(token.type==TOKEN_BRACE2){
            break;
        }else if(token.type==TOKEN_IF){
            getIfState(parser,clist,env);
        }else if(token.type==TOKEN_WHILE){
            getWhileState(parser,clist,env);
        }else{
            ORI_RET()
            if(!getAssignment(parser,clist,env)){
                getExpression(parser,clist,0,env);
                matchToken(parser,TOKEN_SEMI,"\";\"",msgStart);
            }
        }
        part.end=parser->ptr;
        parser->curPart=parser->partList.count;
        LIST_ADD(parser->partList,PartMsg,part)
    }
    addCmd(parser,clist,OPCODE_FREE_VALS);
}
void getIfState(Parser*parser,intList*clist,Env env){
    int msgStart=parser->ptr;
    Token token;
    int lastJump;
    ORI_DEF()
    intList endList;
    LIST_INIT(endList,int)
    matchToken(parser,TOKEN_PARE1,"\"(\"",msgStart);
    getExpression(parser,clist,0,env);
    matchToken(parser,TOKEN_PARE2,"\")\"",msgStart);
    addCmd1(parser,clist,OPCODE_JUMP_IF_FALSE,0);
    lastJump=clist->count-1;
    getBlock(parser,clist,env);
    ORI_ASI()
    msgStart=parser->ptr;
    token=nextToken(parser);
    while(token.type==TOKEN_ELIF){
        addCmd1(parser,clist,OPCODE_JUMP,0);
        LIST_ADD(endList,int,clist->count-1)
        clist->vals[lastJump]=clist->count;
        matchToken(parser,TOKEN_PARE1,"\"(\"",msgStart);
        getExpression(parser,clist,0,env);
        matchToken(parser,TOKEN_PARE2,"\")\"",msgStart);
        addCmd1(parser,clist,OPCODE_JUMP_IF_FALSE,0);
        getBlock(parser,clist,env);
        lastJump=clist->count-1;
        ORI_ASI()
        msgStart=parser->ptr;
        token=nextToken(parser);
    }
    if(token.type==TOKEN_ELSE){
        addCmd1(parser,clist,OPCODE_JUMP,0);
        LIST_ADD(endList,int,clist->count-1)
        clist->vals[lastJump]=clist->count;
        getBlock(parser,clist,env);
    }else{
        clist->vals[lastJump]=clist->count;
        ORI_RET()
    }
    for(int i=0;i<endList.count;i++){
        clist->vals[endList.vals[i]]=clist->count;
    }
}
void getWhileState(Parser*parser,intList*clist,Env env){
    Token token;
    int jump,jret;
    int msgStart=parser->ptr;
    intList breakList;
    LIST_INIT(breakList,int)
    
    matchToken(parser,TOKEN_PARE1,"\"(\"",msgStart);
    jret=clist->count;
    getExpression(parser,clist,0,env);
    matchToken(parser,TOKEN_PARE2,"\")\"",msgStart);
    addCmd1(parser,clist,OPCODE_JUMP_IF_FALSE,0);
    jump=clist->count-1;
    getBlock(parser,clist,env);
    addCmd1(parser,clist,OPCODE_JUMP,jret);
    clist->vals[jump]=clist->count;
}