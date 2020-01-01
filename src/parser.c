#include"parser.h"
const int SYMBOL_COUNT=36;
const int KEYWORD_COUNT=18;
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
    {TOKEN_PERCENT,"%",1},
    {TOKEN_COLON,":",1}
};
const Keyword keywordList[]={
    {TOKEN_FUNC,"func","函数"},
    {TOKEN_WHILE,"while","当"},
    {TOKEN_FOR,"for","循环"},
    {TOKEN_BREAK,"break","结束"},
    {TOKEN_RETURN,"return","返回"},
    {TOKEN_IF,"if","如果"},
    {TOKEN_ELIF,"elif","若"},
    {TOKEN_ELSE,"else","否则"},
    {TOKEN_CAND,"and","且"},
    {TOKEN_COR,"or","或"},
    {TOKEN_CLASS,"class","类"},
    {TOKEN_CAND,"and","且"},
    {TOKEN_COR,"or","或"},
    {TOKEN_IMPORT,"import","引用"},
    {TOKEN_INCLUDE,"include","包括"},
    {TOKEN_TRUE,"true","真"},
    {TOKEN_FALSE,"false","假"},
    {TOKEN_LINE,"__LINE__","__行__"}
};
const int OPERAT_PREFIX_COUNT=3;
const Operat operatPrefix[]={
    {TOKEN_EXCL,OPCODE_NOT,130},
    {TOKEN_INVERT,OPCODE_INVERT,180},
    {TOKEN_SUB,OPCODE_SUBS,100}
};
const int OPERAT_INFIX_COUNT=OPT_METHOD_COUNT;
const Operat operatInfix[]={
    {TOKEN_ADD,OPCODE_ADD,100},
    {TOKEN_SUB,OPCODE_SUB,100},
    {TOKEN_MUL,OPCODE_MUL,120},
    {TOKEN_DIV,OPCODE_DIV,120},
    {TOKEN_CAND,OPCODE_CAND,30},
    {TOKEN_COR,OPCODE_COR,30},
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
extern OpcodeMsg opcodeList[];
/*Token*/
Token nextToken(Parser*parser){
    char word[1024];
    Msg msg;
    Token token;
    bool isFound;
    msg.fileName=parser->fileName;
    msg.code=parser->code;
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
                    reportMsg(msg);
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
                reportMsg(msg);
            }
            word[i]=c;
            parser->column++;
            c=parser->code[++parser->ptr];
        }
        word[i]='\0';
        parser->column+=i;
        token.word=(char*)malloc(i+1);        strcpy(token.word,word);
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
                reportMsg(msg);
            }
            if(i>=1023){
                msg.end=parser->ptr;
                strcpy(msg.text,"the name is too long.");
                reportMsg(msg);
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
    }else if(c=='\"'){
        int i=0;
        msg.line=parser->line;
        msg.column=parser->column;
        msg.start=parser->ptr;
        msg.type=MSG_ERROR;
        c=parser->code[++parser->ptr];
        for(i=0;c!='\"';i++){
            if(c=='\0'){
                if(parser->ptr-msg.start>20){
                    msg.end=msg.start+20;
                }else{
                    msg.end=parser->ptr;
                }
                strcpy(msg.text,"expected \" after the string.");
                reportMsg(msg);
            }
            if(i>=1023){
                msg.end=parser->ptr;
                strcpy(msg.text,"the string is too long.");
                reportMsg(msg);
            }
            if(c=='\n'){
                parser->line++;
                parser->column=1;
                i--;
                c=parser->code[++parser->ptr];
                continue;
            }
            if(c=='\\'){
                switch(parser->code[parser->ptr+1]){
                    case 'n':
                        c='\n';
                        parser->ptr++;
                        break;
                    case '\\':
                        c='\\';
                        parser->ptr++;
                        break;
                    case '0':
                        c='\0';
                        parser->ptr++;
                        break;
                    case '\'':
                        c='\'';
                        parser->ptr++;
                        break;
                    case '\"':
                        c='\"';
                        parser->ptr++;
                        break;
                    default:
                        break;
                }
            }
            word[i]=c;
            c=parser->code[++parser->ptr];
            parser->column++;
        }
        parser->ptr++;
        word[i]='\0';
        token.word=(char*)malloc(i+1);
        strcpy(token.word,word);
        token.type=TOKEN_STRING;
        return token;
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
        msg.fileName=parser->fileName;
        msg.code=parser->code;
        msg.type=MSG_ERROR;
        msg.line=parser->line;
        msg.column=parser->column;
        msg.start=start;
        msg.end=parser->ptr;
        sprintf(msg.text,"expected %s.",str);
        reportMsg(msg);
    }
    return token;
}
/*Token end*/
bool getValue(Parser*parser,intList*clist,Assign*asi,Env env){
    Token token;
    Symbol symbol;
    Assign assign;
    bool canAssign=false;
    char msgText[100];
    ORI_DEF()
    ORI_ASI()
    int msgStart=parser->ptr;
    token=nextToken(parser);
    assign.ncmds.count=0;
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
    }else if(token.type==TOKEN_TRUE){
        symbol.type=SYM_INT;
        symbol.num=1;
        addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
        canAssign=false;
    }else if(token.type==TOKEN_FALSE){
        symbol.type=SYM_INT;
        symbol.num=0;
        addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
        canAssign=false;
    }else if(token.type==TOKEN_LINE){
        symbol.type=SYM_INT;
        symbol.num=parser->line;
        addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
        canAssign=false;
    }else if(token.type==TOKEN_PARE1){
        getExpression(parser,clist,0,env);
        matchToken(parser,TOKEN_PARE2,"\")\"",msgStart);
        canAssign=false;
    }else if(token.type==TOKEN_WORD){
        ORI_ASI()
        char*word=token.word;
        token=nextToken(parser);
        if(token.type==TOKEN_PARE1){
            ORI_ASI()
            symbol.type=SYM_STRING;
            symbol.str=word;
            addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
            int count=0;
            token=nextToken(parser);
            bool needArg=false;
            while(token.type!=TOKEN_PARE2 || needArg){
                ORI_RET()
                getExpression(parser,clist,0,env);
                count++;
                ORI_ASI()
                token=nextToken(parser);
                if(token.type==TOKEN_COMMA){
                    ORI_ASI()
                    token=nextToken(parser);
                    needArg=true;
                }else{
                    needArg=false;
                }
            }
            addCmd1(parser,clist,OPCODE_CALL_FUNCTION,count);
            canAssign=false;
        }else{
            ORI_RET()
            bool isFound=false;
            if(env.classDef>=0){
                for(int i=0;i<parser->classList.vals[env.classDef].var.count;i++){
                    if(strcmp(word,parser->classList.vals[env.classDef].var.vals[i])==0){
                        if(env.isClassVarDef){
                            sprintf(msgText,"the member \"%s\" in the class \"%s\" has already existed.",word,parser->classList.vals[env.classDef].name);
                            reportError(parser,msgText,msgStart);
                        }
                        isFound=true;
                        break;
                    }
                }
            }
            if(!isFound && env.isClassVarDef){
                char*wd=(char*)malloc(strlen(word)+1);
                strcpy(wd,word);
                LIST_ADD(parser->classList.vals[env.classDef].var,Name,wd)
                isFound=true;
                assign.ncmds.count=0;
            }
            symbol.type=SYM_STRING;
            symbol.str=word;
            assign.gcmds.code[0]=OPCODE_LOAD_VAL;
            assign.gcmds.code[1]=addSymbol(parser,symbol);
            assign.gcmds.count=2;
            assign.acmds.code[0]=OPCODE_STORE_VAL;
            assign.acmds.code[1]=assign.gcmds.code[1];
            assign.acmds.count=2;
            if(!isFound){
                assign.ncmds.count=2;
                assign.ncmds.code[0]=OPCODE_PUSH_VAL;
                assign.ncmds.code[1]=assign.gcmds.code[1];
            }
            canAssign=true;
        }
    }else if(token.type==TOKEN_BRACE1){
        int count=0;
        bool needExp=false;
        ORI_ASI()
        token=nextToken(parser);
        while(token.type!=TOKEN_BRACE2 || needExp){
            ORI_RET()
            getExpression(parser,clist,0,env);
            count++;
            ORI_ASI()
            token=nextToken(parser);
            if(token.type==TOKEN_COMMA){
                ORI_ASI()
                token=nextToken(parser);
                needExp=true;
            }else{
                needExp=false;
            }
        }
        addCmd1(parser,clist,OPCODE_MAKE_ARRAY,count);
        canAssign=false;
    }else if(token.type==TOKEN_STRING){
        symbol.type=SYM_STRING;
        symbol.str=token.word;
        addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
        canAssign=false;
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
                addCmds(parser,clist,assign.gcmds);
            }
            token=nextToken(parser);
            if(token.type!=TOKEN_WORD){
                reportError(parser,"expect a class member or method name in expression.",msgStart);
            }
            symbol.type=SYM_STRING;
            symbol.str=token.word;
            ORI_ASI()
            token=nextToken(parser);
            if(token.type==TOKEN_PARE1){
                int count=0;
                addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
                ORI_ASI()
                token=nextToken(parser);
                bool needArg=false;
                while(token.type!=TOKEN_PARE2 || needArg){
                    ORI_RET()
                    getExpression(parser,clist,0,env);
                    count++;
                    ORI_ASI()
                    token=nextToken(parser);
                    if(token.type==TOKEN_COMMA){
                        ORI_ASI()
                        token=nextToken(parser);
                        needArg=true;
                    }else{
                        needArg=false;
                    }
                }
                addCmd1(parser,clist,OPCODE_CALL_METHOD,count);
                canAssign=false;
            }else{
                ORI_RET()
                assign.gcmds.code[0]=OPCODE_LOAD_ATTR;
                assign.gcmds.code[1]=addSymbol(parser,symbol);
                assign.gcmds.count=2;
                assign.acmds.code[0]=OPCODE_STORE_ATTR;
                assign.acmds.code[1]=assign.gcmds.code[1];
                assign.acmds.count=2;
                canAssign=true;
            }
            assign.ncmds.count=0;
        }else if(token.type==TOKEN_BRACKET1){
            if(env.isClassVarDef){
                reportError(parser,"uninitialized member",msgStart);
            }
            if(canAssign){
                addCmds(parser,clist,assign.gcmds);
            }
            canAssign=true;
            getExpression(parser,clist,0,env);
            matchToken(parser,TOKEN_BRACKET2,"\"]\"",msgStart);
            assign.gcmds.code[0]=OPCODE_LOAD_INDEX;
            assign.gcmds.count=1;
            assign.acmds.code[0]=OPCODE_STORE_INDEX;
            assign.acmds.count=1;
            assign.ncmds.count=0;
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
    if(!canAssign && asi!=NULL){
        //reportError(parser,"assignment unsupported.",msgStart);
        return false;
    }
    if(canAssign && asi!=NULL){
        asi->gcmds=assign.gcmds;
        asi->acmds=assign.acmds;
        asi->ncmds=assign.ncmds;
    }else if(canAssign){
        addCmds(parser,clist,assign.gcmds);
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
    bool isVal=getValue(parser,clist,NULL,env);
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
        if(!getValue(parser,&assign.clist,&assign,env)){
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
        if(env.isClassVarDef){
            return true;
        }
        /*全部强制创建变量*/
        for(int i=0;i<assignList.count;i++){
            assign=assignList.vals[i];
            symbol.type=SYM_INT;
            symbol.num=0;
            addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
            LIST_DELETE(assign.clist)
            if(assign.ncmds.count<=0){
                reportError(parser,"can not create the variable.",msgStart);
            }
            addCmds(parser,clist,assign.ncmds);
        }
        return true;
    }else if(token.type==TOKEN_EQUAL){
        opcode=OPCODE_NOP;
    }else if(token.type==TOKEN_ADD_EQUAL){
        opcode=OPCODE_ADD;
    }else if(token.type==TOKEN_SUB_EQUAL){
        opcode=OPCODE_SUB;
    }else if(token.type==TOKEN_MUL_EQUAL){
        opcode=OPCODE_MUL;
    }else if(token.type==TOKEN_DIV_EQUAL){
        opcode=OPCODE_DIV;
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
        int offset=0;
        LIST_CONNECT((*clist),assign.clist,int,0)
        if(opcode!=OPCODE_NOP){
            if(assign.clist.count>0){
                if(assign.acmds.code[0]==OPCODE_STORE_INDEX){
                    addCmd1(parser,clist,OPCODE_STACK_COPY,1);
                    addCmd1(parser,clist,OPCODE_STACK_COPY,1);
                    offset=3;
                }else{
                    addCmd1(parser,clist,OPCODE_STACK_COPY,0);
                    offset=2;
                }
            }else{
                offset=1;
            }
            addCmds(parser,clist,assign.gcmds);
            addCmd1(parser,clist,OPCODE_STACK_COPY,offset);
            addCmd(parser,clist,opcode);
            if(assign.clist.count>0){
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
                addCmd1(parser,clist,OPCODE_POP_STACK,1);
            }
        }else{
            addCmds(parser,clist,assign.acmds);
        }
        LIST_DELETE(assign.clist);
    }
    if(token.type==TOKEN_COMMA){
        reportError(parser,"too many assignments.",msgStart);
    }
    LIST_DELETE(assignList)
    return true;
}
bool checkFile(Parser*parser,char*fileName){
    for(int i=0;i<parser->moduleList.count;i++){
        if(strcmp(fileName,parser->moduleList.vals[i].name)==0){
            return true;
        }
    }
    return false;
}
void getBlock(Parser*parser,intList*clist,Env env){
    Token token;
    Symbol symbol;
    int msgStart=parser->ptr;
    Part part;
    part.code=parser->code;
    part.fileName=parser->fileName;
    bool isGlobal=env.isGlobal;
    env.isGlobal=false;
    char temp[50];
    ORI_DEF()
    if(!isGlobal){
        matchToken(parser,TOKEN_BRACE1,"\"{\"",msgStart);
        addCmd(parser,clist,OPCODE_SET_FIELD);
    }
    while(1){
        part.line=parser->line;
        part.column=parser->column;
        part.start=parser->ptr;
        msgStart=parser->ptr;
        ORI_ASI()
        token=nextToken(parser);
        if(token.type==TOKEN_END){
            if(isGlobal){
                break;
            }else{
                reportError(parser,"expected \"}\" after a block.",msgStart);
            }
        }else if(token.type==TOKEN_BRACE2 && !isGlobal){
            break;
        }else if(token.type==TOKEN_BREAK){
            if(env.breakList==NULL){
                reportError(parser,"useless break.",msgStart);
            }
            addCmd1(parser,clist,OPCODE_JUMP,0);
            LIST_ADD((*env.breakList),int,clist->count-1)
        }else if(token.type==TOKEN_RETURN){
            if(!env.isFuncDef){
                reportError(parser,"useless return.",msgStart);
            }
            parser->curPart=parser->partList.count;
            getExpression(parser,clist,0,env);
            addCmd(parser,clist,OPCODE_RETURN);
            matchToken(parser,TOKEN_SEMI,"\";\"",msgStart);
            part.end=parser->ptr;
            LIST_ADD(parser->partList,Part,part)
        }else if(token.type==TOKEN_IF){
            getIfState(parser,clist,env);
        }else if(token.type==TOKEN_FUNC){
            getFunction(parser,clist,env);
        }else if(token.type==TOKEN_WHILE){
            getWhileState(parser,clist,env);
        }else if(token.type==TOKEN_FOR){
            getForState(parser,clist,env);
        }else if(token.type==TOKEN_CLASS){
            getClass(parser,clist,env);
        }else if(token.type==TOKEN_INCLUDE){
            char fileName[MAX_WORD_LENGTH];
            char*t;
            token=nextToken(parser);
            if(token.type==TOKEN_WORD){
                strcpy(fileName,token.word);
                free(token.word);
                if(!checkFile(parser,fileName)){
                    strcat(fileName,FILE_POSTFIX);
                    compile(parser,fileName,parser->isLib);
                }
            }else if(token.type==TOKEN_STRING){
                t=cutPostfix(token.word);
                if(!checkFile(parser,t)){
                    compile(parser,token.word,parser->isLib);
                }else{
                    free(token.word);
                }
                free(t);
            }else{
                reportError(parser,"expected a code file name",msgStart);
            }
            matchToken(parser,TOKEN_SEMI,"\";\"",msgStart);
        }else if(token.type==TOKEN_IMPORT){
            char*fileName=NULL;
            token=nextToken(parser);
            if(token.type==TOKEN_WORD){
                fileName=(char*)malloc(strlen(parser->path)+strlen(token.word)+strlen(FILE_LIB_POSTFIX)+5);
                sprintf(fileName,"%s/lib/%s%s",parser->path,token.word,FILE_LIB_POSTFIX);
            }else if(token.type==TOKEN_STRING){
                fileName=token.word;
            }else{
                reportError(parser,"expected a library file name",msgStart);
            }
            import(parser,fileName);
            free(fileName);
            matchToken(parser,TOKEN_SEMI,"\";\"",msgStart);
        }else if(token.type==TOKEN_RIGHT){
            token=matchToken(parser,TOKEN_WORD,"a operation code",msgStart);
            OpcodeMsg opcode=opcodeList[0];
            bool isFound=false;
            for(int i=0;i<OPCODE_COUNT;i++){
                if(strcmp(token.word,opcodeList[i].name)==0){
                    opcode=opcodeList[i];
                    isFound=true;
                    free(token.word);
                    break;
                }
            }
            if(!isFound){
                sprintf(temp,"operation code \"%s\" not found.",token.word);
                reportError(parser,temp,msgStart);
            }
            parser->curPart=parser->partList.count;
            if(opcode.isArg){
                if(opcode.isSymbol){
                    token=nextToken(parser);
                    switch(token.type){
                        case TOKEN_INTEGER:
                            symbol.type=SYM_INT;
                            symbol.num=token.num;
                            break;
                        case TOKEN_FLOAT:
                            symbol.type=SYM_FLOAT;
                            symbol.numf=token.numf;
                            break;
                        case TOKEN_STRING:
                        case TOKEN_WORD:
                            symbol.type=SYM_STRING;
                            symbol.str=token.word;
                            break;
                        default:
                            reportError(parser,"unsupported symbol for operation number.",msgStart);
                            break;
                    }
                    addCmd1(parser,clist,opcode.opcode,addSymbol(parser,symbol));
                }else{
                    token=matchToken(parser,TOKEN_INTEGER,"operation number",msgStart);
                    addCmd1(parser,clist,opcode.opcode,token.num);
                }
            }else{
                addCmd(parser,clist,opcode.opcode);
            }
            matchToken(parser,TOKEN_SEMI,"\";\" after operation code",msgStart);
            part.end=parser->ptr;
            LIST_ADD(parser->partList,Part,part)
        }else{
            ORI_RET()
            parser->curPart=parser->partList.count;
            ORI_ASI()
            if(!getAssignment(parser,clist,env)){
                ORI_RET()
                getExpression(parser,clist,0,env);
                matchToken(parser,TOKEN_SEMI,"\";\"",msgStart);
                addCmd1(parser,clist,OPCODE_POP_STACK,1);
            }
            part.end=parser->ptr;
            LIST_ADD(parser->partList,Part,part)
        }
    }
    if(!isGlobal){
        addCmd(parser,clist,OPCODE_FREE_FIELD);
    }else{
        addCmd(parser,clist,OPCODE_NOP);
    }
}
void getIfState(Parser*parser,intList*clist,Env env){
    int msgStart=parser->ptr;
    Token token;
    int lastJump;
    Part part;
    part.code=parser->code;
    part.fileName=parser->fileName;
    ORI_DEF()
    intList endList;
    LIST_INIT(endList,int)
    part.line=parser->line;
    part.column=parser->column;
    part.start=parser->ptr;
    matchToken(parser,TOKEN_PARE1,"\"(\"",msgStart);
    getExpression(parser,clist,0,env);
    matchToken(parser,TOKEN_PARE2,"\")\"",msgStart);
    parser->curPart=parser->partList.count;
    LIST_ADD(parser->partList,Part,part)
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
        part.line=parser->line;
        part.column=parser->column;
        part.start=parser->ptr;
        matchToken(parser,TOKEN_PARE1,"\"(\"",msgStart);
        getExpression(parser,clist,0,env);
        matchToken(parser,TOKEN_PARE2,"\")\"",msgStart);
        parser->curPart=parser->partList.count;
        LIST_ADD(parser->partList,Part,part)
        addCmd1(parser,clist,OPCODE_JUMP_IF_FALSE,0);
        lastJump=clist->count-1;
        getBlock(parser,clist,env);
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
    int jump,jret;
    int msgStart=parser->ptr;
    intList breakList;
    Part part;
    part.code=parser->code;
    part.fileName=parser->fileName;
    int pt;
    part.line=parser->line;
    part.column=parser->column;
    part.start=parser->ptr;
    LIST_INIT(breakList,int)
    env.breakList=&breakList;
    addCmd(parser,clist,OPCODE_SET_LOOP);/*用前面的part*/
    matchToken(parser,TOKEN_PARE1,"\"(\"",msgStart);
    jret=clist->count;
    getExpression(parser,clist,0,env);
    matchToken(parser,TOKEN_PARE2,"\")\"",msgStart);
    part.end=parser->ptr;
    pt=parser->partList.count;
    parser->curPart=pt;
    LIST_ADD(parser->partList,Part,part)
    addCmd1(parser,clist,OPCODE_JUMP_IF_FALSE,0);
    jump=clist->count-1;
    getBlock(parser,clist,env);
    parser->curPart=pt;
    addCmd1(parser,clist,OPCODE_JUMP,jret);
    clist->vals[jump]=clist->count;
    for(int i=0;i<breakList.count;i++){
        clist->vals[breakList.vals[i]]=clist->count;
    }
    addCmd(parser,clist,OPCODE_FREE_LOOP);
    LIST_DELETE(breakList)
}
void getFunction(Parser*parser,intList*clist,Env env){
    Func func;
    Symbol symbol;
    Part part;
    part.code=parser->code;
    part.fileName=parser->fileName;
    part.start=parser->ptr;
    part.line=parser->line;
    part.column=parser->column;
    int msgStart=parser->ptr;
    Token token=matchToken(parser,TOKEN_WORD,"a function name",msgStart);
    func.name=token.word;
    LIST_INIT(func.clist,int)
    matchToken(parser,TOKEN_PARE1,"\"(\" when defining a function",msgStart);
    token=nextToken(parser);
    LIST_INIT(func.args,Name)
    bool needArg=false;
    while(token.type!=TOKEN_PARE2 || needArg){
        if(token.type!=TOKEN_WORD){
            reportError(parser,"expected an argument when defining a function.",msgStart);
        }
        LIST_ADD(func.args,Name,token.word)
        token=nextToken(parser);
        if(token.type==TOKEN_COMMA){
            token=nextToken(parser);
            needArg=true;
        }else{
            needArg=false;
        }
    }
    part.end=parser->ptr;
    env.isFuncDef=true;
    env.breakList=NULL;
    getBlock(parser,&func.clist,env);
    parser->curPart=parser->partList.count;
    LIST_ADD(parser->partList,Part,part)
    symbol.type=SYM_INT;
    symbol.num=0;
    addCmd1(parser,&func.clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
    addCmd(parser,&func.clist,OPCODE_RETURN);
    addCmd1(parser,clist,OPCODE_ENABLE_FUNCTION,parser->funcList.count);
    func.moduleID=parser->curModule;
    LIST_ADD(parser->funcList,Func,func)
}
void getClass(Parser*parser,intList*clist,Env env){
    Func method;
    Token token;
    Class classd;
    int class;
    Symbol symbol;
    Part part;
    part.code=parser->code;
    part.fileName=parser->fileName;
    int tpt;
    char*initName,*destroyName;
    ORI_DEF()
    int msgStart=parser->ptr;
    part.start=parser->ptr;
    part.line=parser->line;
    part.column=parser->column;
    token=matchToken(parser,TOKEN_WORD,"class name",msgStart);
    classd.name=token.word;
    initName=(char*)malloc(5+strlen(classd.name));
    sprintf(initName,"init%s",classd.name);
    destroyName=(char*)malloc(8+strlen(classd.name));
    sprintf(destroyName,"destroy%s",classd.name);
    for(int i=0;i<OPT_METHOD_COUNT;i++){
        classd.optID[i]=-1;
    }
    classd.initID=-1;
    classd.destroyID=-1;
    classd.initValID=-1;
    LIST_INIT(classd.methods,Func)
    LIST_INIT(classd.var,Name)
    method.moduleID=parser->curModule;
    LIST_INIT(classd.parentList,int);
    if(strcmp(classd.name,"meta")!=0){
        LIST_ADD(classd.parentList,int,CLASS_META)
    }
    token=nextToken(parser);
    if(token.type==TOKEN_COLON){
        symbol.type=SYM_STRING;
        while(1){
            token=matchToken(parser,TOKEN_WORD,"a class to extend",msgStart);
            symbol.str=token.word;
            addCmd1(parser,clist,OPCODE_LOAD_VAL,addSymbol(parser,symbol));
            addCmd1(parser,clist,OPCODE_EXTEND_CLASS,parser->classList.count);
            token=nextToken(parser);
            if(token.type==TOKEN_BRACE1){
                break;
            }else if(token.type==TOKEN_COMMA){
                continue;
            }else{
                reportError(parser,"expected \"{\" or \",\" when defining a class.",msgStart);
            }
        }
    }
    LIST_ADD(parser->classList,Class,classd)
    class=parser->classList.count-1;
    part.end=parser->ptr;
    tpt=parser->partList.count;
    parser->curPart=tpt;
    LIST_ADD(parser->partList,Part,part)
    env.classDef=class;
    env.isGlobal=false;
    env.breakList=NULL;
    while(1){
        ORI_ASI()
        msgStart=parser->ptr;
        token=nextToken(parser);
        if(token.type==TOKEN_BRACE2){
            break;
        }else if(token.type==TOKEN_END){
            reportError(parser,"expected \"}\" after defining a class.",msgStart);
        }else if(token.type==TOKEN_FUNC){
            env.isFuncDef=true;
            part.start=parser->ptr;
            part.line=parser->line;
            part.column=parser->column;
            token=matchToken(parser,TOKEN_WORD,"a method name",msgStart);
            bool isOpt=false;
            method.name=token.word;
            if(strcmp(method.name,"operator")==0){
                isOpt=true;
            }else if(strcmp(method.name,initName)==0){
                parser->classList.vals[class].initID=parser->classList.vals[class].methods.count;
            }else if(strcmp(method.name,destroyName)==0){
                parser->classList.vals[class].destroyID=parser->classList.vals[class].methods.count;
            }
            for(int i=0;i<parser->classList.vals[class].methods.count && !isOpt;i++){
                if(strcmp(method.name,parser->classList.vals[class].methods.vals[i].name)==0){
                    reportError(parser,"the method has already existed.",msgStart);
                }
            }
            LIST_INIT(method.args,Name)
            LIST_INIT(method.clist,int)
            if(isOpt){
                bool isFound=false;
                token=nextToken(parser);
                for(int i=0;i<OPERAT_INFIX_COUNT;i++){
                    if(token.type==operatInfix[i].tokenType){
                        if(parser->classList.vals[class].optID[operatInfix[i].opcode]>=0){
                            reportError(parser,"the operator method has already exited.",msgStart);
                        }
                        parser->classList.vals[class].optID[operatInfix[i].opcode]=parser->classList.vals[class].methods.count;
                        isFound=true;
                    }
                }
                if(!isFound){
                    reportError(parser,"expected an operator.",msgStart);
                }
            }
            matchToken(parser,TOKEN_PARE1,"\"(\"",msgStart);
            token=nextToken(parser);
            bool needArg=false;
            while(token.type!=TOKEN_PARE2 || needArg){
                if(token.type!=TOKEN_WORD){
                    reportError(parser,"expected an argument when defining a function.",msgStart);
                }
                LIST_ADD(method.args,Name,token.word)
                token=nextToken(parser);
                if(token.type==TOKEN_COMMA){
                    token=nextToken(parser);
                    needArg=true;
                }else{
                    needArg=false;
                }
            }
            if(isOpt && method.args.count!=1){
                reportWarning(parser,"operator method can only support 1 argument.",msgStart);
            }
            part.end=parser->ptr;
            int pt=parser->partList.count;
            LIST_ADD(parser->partList,Part,part)
            getBlock(parser,&method.clist,env);
            parser->curPart=pt;
            symbol.type=SYM_INT;
            symbol.num=0;
            addCmd1(parser,&method.clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
            addCmd(parser,&method.clist,OPCODE_RETURN);
            method.moduleID=parser->curModule;
            LIST_ADD(parser->classList.vals[class].methods,Func,method)
            env.isFuncDef=false;
        }else{
            ORI_RET()
            env.isClassVarDef=true;
            parser->curPart=parser->partList.count;
            if(parser->classList.vals[class].initValID<0){
                Func func;
                func.name=(char*)malloc(5);
                strcpy(func.name,"init");
                LIST_INIT(func.args,Name)
                LIST_INIT(func.clist,int)
                func.moduleID=parser->curModule;
                parser->classList.vals[class].initValID=parser->classList.vals[class].methods.count;
                LIST_ADD(parser->classList.vals[class].methods,Func,func)
            }
            if(!getAssignment(parser,&parser->classList.vals[class].methods.vals[parser->classList.vals[class].initValID].clist,env)){
                reportError(parser,"unsupported expression in class.",msgStart);
            }
            part.end=parser->ptr;
            LIST_ADD(parser->partList,Part,part)
            env.isClassVarDef=false;
        }
    }
    parser->curPart=tpt;
    classd=parser->classList.vals[class];
    if(classd.initValID>=0){
        symbol.type=SYM_INT;
        symbol.num=0;
        addCmd1(parser,&parser->classList.vals[class].methods.vals[classd.initValID].clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
        addCmd(parser,&parser->classList.vals[class].methods.vals[classd.initValID].clist,OPCODE_RETURN);
    }
    Func initFunc;
    initFunc.moduleID=parser->curModule;
    if(classd.initID>=0){
        initFunc.args=classd.methods.vals[classd.initID].args;
    }else{
        LIST_INIT(initFunc.args,Name)
    }
    LIST_INIT(initFunc.clist,int)
    addCmd1(parser,&initFunc.clist,OPCODE_MAKE_OBJECT,class);
    addCmd(parser,&initFunc.clist,OPCODE_RETURN);
    addCmd1(parser,clist,OPCODE_ENABLE_FUNCTION,parser->funcList.count);
    addCmd1(parser,clist,OPCODE_ENABLE_CLASS,class);
    initFunc.name=classd.name;
    LIST_ADD(parser->funcList,Func,initFunc)
    parser->classList.vals[class].varBasis=classd.var.count;
    free(initName);
    free(destroyName);
}
void getForState(Parser*parser,intList*clist,Env env){
    int msgStart=parser->ptr;
    Part part;
    intList breakList;
    Token token;
    Symbol symbol;
    part.code=parser->code;
    part.fileName=parser->fileName;
    part.line=parser->line;
    part.column=parser->column;
    part.start=parser->ptr;
    int pt=parser->partList.count;
    parser->curPart=pt;
    LIST_INIT(breakList,int)
    env.breakList=&breakList;
    matchToken(parser,TOKEN_PARE1,"\"(\" in for state",msgStart);
    addCmd(parser,clist,OPCODE_SET_LOOP);
    token=matchToken(parser,TOKEN_WORD,"a for state variable",msgStart);
    int s0,st;
    symbol.type=SYM_INT;
    symbol.num=0;
    s0=addSymbol(parser,symbol);
    addCmd1(parser,clist,OPCODE_LOAD_CONST,s0);
    symbol.type=SYM_STRING;
    symbol.str=token.word;
    st=addSymbol(parser,symbol);
    addCmd1(parser,clist,OPCODE_PUSH_VAL,st);
    matchToken(parser,TOKEN_COMMA,"\",\" in for state",msgStart);
    getExpression(parser,clist,0,env);
    matchToken(parser,TOKEN_PARE2,"\")\" in for state",msgStart);
    part.end=parser->ptr;
    LIST_ADD(parser->partList,Part,part)
    addCmd1(parser,clist,OPCODE_LOAD_CONST,s0);
    int jret=clist->count;
    addCmd1(parser,clist,OPCODE_STACK_COPY,1);
    addCmd(parser,clist,OPCODE_GET_VARCOUNT);
    addCmd1(parser,clist,OPCODE_STACK_COPY,1);
    addCmd(parser,clist,OPCODE_GTHAN);
    addCmd1(parser,clist,OPCODE_JUMP_IF_FALSE,0);
    int jptr=clist->count-1;
    addCmd1(parser,clist,OPCODE_STACK_COPY,1);
    addCmd1(parser,clist,OPCODE_STACK_COPY,1);
    addCmd(parser,clist,OPCODE_LOAD_INDEX);
    addCmd1(parser,clist,OPCODE_STORE_VAL,st);
    symbol.type=SYM_INT;
    symbol.num=1;
    addCmd1(parser,clist,OPCODE_LOAD_CONST,addSymbol(parser,symbol));
    addCmd(parser,clist,OPCODE_ADD);
    getBlock(parser,clist,env);
    addCmd1(parser,clist,OPCODE_JUMP,jret);
    for(int i=0;i<breakList.count;i++){
        clist->vals[breakList.vals[i]]=clist->count;
    }
    clist->vals[jptr]=clist->count;
    addCmd1(parser,clist,OPCODE_POP_STACK,2);
    addCmd1(parser,clist,OPCODE_POP_VAR,1);
    addCmd(parser,clist,OPCODE_FREE_LOOP);
    LIST_DELETE(breakList)
}