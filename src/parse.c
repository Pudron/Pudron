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
        }else if(strcmp(token.word,"putc")==0){
            token.type=TOKEN_PUTC;
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
        if(parser->code[parser->ptr+1]=='='){
            token.type=TOKEN_ADD_EQUAL;
            parser->ptr+=2;
            return token;
        }
        token.type=TOKEN_ADD;
        parser->ptr++;
        return token;
    }else if(c=='-'){
        if(parser->code[parser->ptr+1]=='='){
            token.type=TOKEN_SUB_EQUAL;
            parser->ptr+=2;
            return token;
        }
        token.type=TOKEN_SUB;
        parser->ptr++;
        return token;
    }else if(c=='*'){
        if(parser->code[parser->ptr+1]=='='){
            token.type=TOKEN_MUL_EQUAL;
            parser->ptr+=2;
            return token;
        }
        token.type=TOKEN_MUL;
        parser->ptr++;
        return token;
    }else if(c=='/'){
        if(parser->code[parser->ptr+1]=='='){
            token.type=TOKEN_DIV_EQUAL;
            parser->ptr+=2;
            return token;
        }
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
    }else if(c=='~'){
        token.type=TOKEN_INVERT;
        parser->ptr++;
        return token;
    }else if(c=='['){
        token.type=TOKEN_BRACKET1;
        parser->ptr++;
        return token;
    }else if(c==']'){
        token.type=TOKEN_BRACKET2;
        parser->ptr++;
        return token;
    }else if(c=='!'){
        if(parser->code[parser->ptr+1]=='='){
            token.type=TOKEN_NOT_EQUAL;
            parser->ptr+=2;
            return token;
        }
        token.type=TOKEN_EXCL;
        parser->ptr++;
        return token;
    }else if(c=='>'){
        if(parser->code[parser->ptr+1]=='='){
            token.type=TOKEN_GTHAN_EQUAL;
            parser->ptr+=2;
            return token;
        }
        if(parser->code[parser->ptr+1]=='>'){
            if(parser->code[parser->ptr+2]=='='){
                token.type=TOKEN_RIGHT_EQUAL;
                parser->ptr+=3;
                return token;
            }
            token.type=TOKEN_RIGHT;
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
        if(parser->code[parser->ptr+1]=='<'){
            if(parser->code[parser->ptr+2]=='='){
                token.type=TOKEN_LEFT_EQUAL;
                parser->ptr+=3;
                return token;
            }
            token.type=TOKEN_LEFT;
            parser->ptr+=2;
            return token;
        }
        token.type=TOKEN_LTHAN;
        parser->ptr++;
        return token;
    }else if(c=='&'){
        if(parser->code[parser->ptr+1]=='='){
            token.type=TOKEN_AND_EQUAL;
            parser->ptr+=2;
            return token;
        }
        token.type=TOKEN_AND;
        parser->ptr++;
        return token;
    }else if(c=='|'){
        if(parser->code[parser->ptr+1]=='='){
            token.type=TOKEN_OR_EQUAL;
            parser->ptr+=2;
            return token;
        }
        token.type=TOKEN_OR;
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
bool getExpression(Parser*parser,CmdList*clist,ReturnType*rtype,Environment envirn){
    Token token;
    OperatList olist;
    Operat operat;
    char msg[100];
    Variable var;
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
        }else if(token.type==TOKEN_EXCL){
            operat.handle_prefix=HANDLE_INVERT;
            token=nextToken(parser);
        }else if(token.type==TOKEN_INVERT){
            operat.handle_prefix=HANDLE_INVERT2;
            token=nextToken(parser);
        }else{
            operat.handle_prefix=HANDLE_NOP;
        }
        operat.rtype.isVar=false;
        operat.rtype.isStack=false;
        if(token.type==TOKEN_PARE1){
            if(!getExpression(parser,clist,&operat.rtype,envirn)){
                reportWarning(parser,"expected an expression in the \"( )\".");
            }
            token=nextToken(parser);
            if(token.type!=TOKEN_PARE2){
                reportError(parser,"expected \")\".");
            }
        }else if(token.type==TOKEN_INTEGER || token.type==TOKEN_FLOAT){
            //addCmd1(clist,HANDLE_PUSH,DATA_INTEGER,token.num);
            addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_INTEGER,REG_AX,token.num);
            operat.rtype.class=(token.type==TOKEN_INTEGER)?TYPE_INTEGER:TYPE_FLOAT;
        }else if(token.type==TOKEN_WORD){
            if(!getVarRef(parser,token.word,clist,&var,envirn)){
                sprintf(msg,"unfound variable \"%s\".",token.word);
                reportError(parser,msg);
            }
            if(var.dim){
                parser->ptr=rptr;
                parser->line=rline;
                return false;
            }
            //addCmd1(clist,HANDLE_PUSH,DATA_REG,REG_AX);
            operat.rtype.isVar=true;
            operat.rtype.class=var.class;
            operat.rtype.dim=var.dim;
            if(var.vtype==VAR_PART){
                operat.rtype.isStack=true;
            }else if(var.vtype==VAR_PARAC){
                operat.rtype.isStack=false;
                addCmd2(clist,HANDLE_POPS,DATA_REG,DATA_REG,REG_AX,REG_AX);
            }else{
                operat.rtype.isStack=false;
            }
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
        }else if(token.type==TOKEN_AND){
            operat.handle_infix=HANDLE_AND;
            operat.power=100;
        }else if(token.type==TOKEN_OR){
            operat.handle_infix=HANDLE_OR;
            operat.power=100;
        }else if(token.type==TOKEN_GTHAN){
            operat.handle_infix=HANDLE_GTHAN;
            operat.power=30;
        }else if(token.type==TOKEN_GTHAN_EQUAL){
            operat.handle_infix=HANDLE_GTHAN_EQUAL;
            operat.power=30;
        }else if(token.type==TOKEN_LTHAN){
            operat.handle_infix=HANDLE_LTHAN;
            operat.power=30;
        }else if(token.type==TOKEN_LTHAN_EQUAL){
            operat.handle_infix=HANDLE_LTHAN_EQUAL;
            operat.power=30;
        }else if(token.type==TOKEN_NOT_EQUAL){
            operat.handle_infix=HANDLE_NOT_EQUAL;
            operat.power=30;
        }else if(token.type==TOKEN_LEFT){
            operat.handle_infix=HANDLE_LEFT;
            operat.power=110;
        }else if(token.type==TOKEN_RIGHT){
            operat.handle_infix=HANDLE_RIGHT;
            operat.power=110;
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
            //addCmd1(clist,HANDLE_POP,DATA_REG,REG_BX);
            addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG,REG_BX,REG_AX);
            if(operat.handle_postfix==HANDLE_ADD){
                if(operat.rtype.isVar){
                    if(operat.rtype.class==TYPE_INTEGER){
                        addCmd2(clist,HANDLE_ADD,DATA_REG_POINTER,DATA_INTEGER,REG_BX,1);
                    }else if(operat.rtype.class==TYPE_FLOAT){
                        addCmd2(clist,HANDLE_FADD,DATA_REG_POINTER,DATA_INTEGER,REG_BX,1);
                    }else{
                        sprintf(msg,"the type \"%s\" does not support postfix calculation.",parser->classList.vals[operat.rtype.class].name);
                        reportWarning(parser,msg);
                    }
                }else{
                    if(operat.rtype.class==TYPE_INTEGER){
                        addCmd2(clist,HANDLE_ADD,DATA_REG,DATA_INTEGER,REG_BX,1);
                    }else if(operat.rtype.class==TYPE_FLOAT){
                        addCmd2(clist,HANDLE_FADD,DATA_REG,DATA_INTEGER,REG_BX,1);
                    }else{
                            reportError(parser,"unknown constant.");
                    }
                }
                operat.handle_postfix=HANDLE_NOP;
            }
            if(operat.rtype.isVar && (operat.rtype.class==TYPE_FLOAT || operat.rtype.class==TYPE_INTEGER)){
                addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG_POINTER,REG_BX,REG_BX);
                operat.rtype.isVar=false;
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
                    if(opt.rtype.isVar){
                        if(opt.rtype.class==TYPE_INTEGER){
                            addCmd2(clist,HANDLE_ADD,DATA_REG_POINTER,DATA_INTEGER,REG_AX,1);
                        }else if(operat.rtype.class==TYPE_FLOAT){
                            addCmd2(clist,HANDLE_FADD,DATA_REG_POINTER,DATA_INTEGER,REG_AX,1);
                        }else{
                            sprintf(msg,"the type \"%s\" does not support postfix calculation.",parser->classList.vals[operat.rtype.class].name);
                            reportWarning(parser,msg);
                        }
                    }else{
                        if(opt.rtype.class==TYPE_INTEGER){
                            addCmd2(clist,HANDLE_ADD,DATA_REG,DATA_INTEGER,REG_AX,1);
                        }else if(opt.rtype.class==TYPE_FLOAT){
                            addCmd2(clist,HANDLE_FADD,DATA_REG,DATA_INTEGER,REG_AX,1);
                        }else{
                            reportError(parser,"unknown constant.");
                        }
                    }
                    opt.handle_postfix=HANDLE_NOP;
                }
                if(opt.rtype.isVar && (opt.rtype.class==TYPE_FLOAT || opt.rtype.class==TYPE_INTEGER)){
                    addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG_POINTER,REG_AX,REG_AX);
                }
                if(opt.handle_prefix!=HANDLE_NOP){
                    addCmd1(clist,opt.handle_prefix,DATA_REG,REG_AX);
                }
                if(opt.rtype.class==TYPE_FLOAT){
                    if(operat.rtype.class!=TYPE_FLOAT && operat.rtype.class!=TYPE_INTEGER){
                        sprintf(msg,"can not cover type \"%s\" to \"%s\".",parser->classList.vals[TYPE_FLOAT].name,parser->classList.vals[operat.rtype.class].name);
                        reportError(parser,msg);
                    }
                    opt.handle_infix=handleFloat(opt.handle_infix);
                }else if(opt.rtype.class==TYPE_INTEGER){
                    if(operat.rtype.class==TYPE_FLOAT){
                        opt.handle_infix=handleFloat(opt.handle_infix);
                    }else if(operat.rtype.class!=TYPE_INTEGER){
                        sprintf(msg,"can not cover type \"%s\" to \"%s\".",parser->classList.vals[TYPE_INTEGER].name,parser->classList.vals[operat.rtype.class].name);
                        reportError(parser,msg);
                    }
                }
                addCmd2(clist,opt.handle_infix,DATA_REG,DATA_REG,REG_AX,REG_BX);
                addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG,REG_BX,REG_AX);
                operat.rtype.class=opt.rtype.class;
                LIST_SUB(olist,Operat);
            }
            //addCmd1(clist,HANDLE_PUSH,DATA_REG,REG_BX);
        }
        if(isEnd){
            rtype->class=operat.rtype.class;
            rtype->isVar=operat.rtype.isVar;
            break;
        }
        addCmd1(clist,HANDLE_PUSH,DATA_REG,REG_AX);
        LIST_ADD(olist,Operat,operat);
    }
    LIST_DELETE(olist);
    return true;
}
bool getVariableDef(Parser*parser,VariableList*vlist,CmdList*clist,bool isPart,int*partSize,Environment envirn){
    Token token;
    Variable var;
    char msg[100];
    intList arraySize;
    int rptr=parser->ptr;
    int rline=parser->line;
    ReturnType rtype;
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
    var.vtype=isPart?VAR_PART:VAR_GLOBAL;
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
        var.subVar=NULL;
        var.dim=0;
        var.unitSize=0;
        var.arrayCount=0;
        strcpy(var.name,token.word);
        var.ptr=isPart?*partSize:parser->dataSize;
        token=nextToken(parser);
        /*处理数组 */
        int dim=0;
        Variable*var2;
        for(int i=0;token.type==TOKEN_BRACKET1;i++){
            if(i==0){
                LIST_INIT(arraySize,int);
            }
            token=nextToken(parser);
            if(token.type!=TOKEN_INTEGER){
                reportError(parser,"expected integer when define array.");
            }
            token=nextToken(parser);
            if(token.type!=TOKEN_BRACKET2){
                reportError(parser,"expected \"]\" when define array.");
            }
            LIST_ADD(arraySize,int,token.num);
            token=nextToken(parser);
            dim++;
        }
        if(dim){
            LIST_ADD(arraySize,int,parser->classList.vals[var.class].size);
            for(int i=arraySize.count-2;i>=0;i--){
                arraySize.vals[i]=arraySize.vals[i+1]*arraySize.vals[i];
            }
            if(var.vtype==VAR_GLOBAL){
                parser->dataSize+=arraySize.vals[0];
            }else{
                (*partSize)+=arraySize.vals[0];
            }
            var2=&var;
            for(int i=0;i<dim;i++){
                var2->vtype=var.vtype;
                var2->ptr=var.ptr;
                var2->unitSize=arraySize.vals[i+1];
                var2->dim=dim-i;
                var2->class=var.class;
                var2->arrayCount=arraySize.vals[i]/arraySize.vals[i+1];
                var2->subVar=(Variable*)malloc(sizeof(Variable));
                var2=var2->subVar;
            }
            var2->dim=0;
            var2->unitSize=0;
            var2->class=var.class;
            LIST_DELETE(arraySize);
        }else{
            if(var.vtype==VAR_GLOBAL){
                parser->dataSize+=parser->classList.vals[var.class].size;
            }else{
                (*partSize)+=parser->classList.vals[var.class].size;
            }
        }
        LIST_ADD((*vlist),Variable,var);
        if(token.type==TOKEN_EQUAL){
            if(var.dim){//will change by partVar
                addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_INTEGER,REG_AX,var.ptr);
                if(!getArray(parser,clist,var,envirn)){
                    reportError(parser,"expected an array when initializing variable.");
                }
            }else{
                if(!getExpression(parser,clist,&rtype,envirn)){
                    reportError(parser,"expected an expression when initializing variable.");
                }
                if((var.class==TYPE_FLOAT && rtype.class==TYPE_INTEGER) || (var.class==TYPE_INTEGER && rtype.class==TYPE_INTEGER) || (var.class==TYPE_FLOAT && rtype.class==TYPE_FLOAT)){
                    //addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
                    if(isPart){
                        addCmd1(clist,HANDLE_PUSH,DATA_REG,REG_AX);
                    }
                    else{
                        addCmd2(clist,HANDLE_MOV,DATA_POINTER,DATA_REG,var.ptr,REG_AX);
                    }
                }else if(var.class==TYPE_INTEGER && rtype.class==TYPE_FLOAT){
                    //addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
                    addCmd2(clist,HANDLE_FTOI,DATA_REG,DATA_REG,REG_AX,REG_AX);
                    if(isPart){
                        addCmd1(clist,HANDLE_PUSH,DATA_REG,REG_AX);
                    }
                    else{
                        addCmd2(clist,HANDLE_MOV,DATA_POINTER,DATA_REG,var.ptr,REG_AX);
                    }
                }else if(var.class!=rtype.class){
                    sprintf(msg,"incompatible types when assigning to type \"%s\" from type \"%s\".",parser->classList.vals[var.class].name,parser->classList.vals[rtype.class].name);
                    reportError(parser,msg);
                }else{
                    /*对象赋值，有待改进 */
                    //addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
                    addCmd2(clist,HANDLE_MOV,DATA_POINTER,DATA_REG,var.ptr,REG_AX);
                }
            }else if(isPart){
                if(var.dim){
                    addCmd2(clist,HANDLE_PUSHB,DATA_INTEGER,DATA_INTEGER,0,var.arrayCount*unitSize);
                }else{
                    addCmd2(clist,HANDLE_PUSHB,DATA_INTEGER,DATA_INTEGER,0,parser->classList.vals[var.class].size);
                }
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
    ReturnType rtype;
    Variable var;
    VariableList  varList;
    int rptr,rline;
    HandleType ht;
    char msg[100];
    int stackPtr=-1;
    rptr=parser->ptr;
    rline=parser->line;
    LIST_INIT(varList,Variable);
    while(1){
        token=nextToken(parser);
        if(token.type!=TOKEN_WORD){
            parser->ptr=rptr;
            parser->line=rline;
            return false;
        }
        if(!getVarRef(parser,token.word,clist,&var,envirn)){
            parser->ptr=rptr;
            parser->line=rline;
            return false;
        }
        addCmd1(clist,HANDLE_PUSH,DATA_REG,REG_AX);
        token=nextToken(parser);
        stackPtr++;
        LIST_ADD(varList,Variable,var);
        if(token.type==TOKEN_COMMA){
            continue;
        }else if(token.type==TOKEN_EQUAL){
            ht=HANDLE_MOV;
            break;
        }else if(token.type==TOKEN_ADD_EQUAL){
            ht=HANDLE_ADD;
            break;
        }else if(token.type==TOKEN_SUB_EQUAL){
            ht=HANDLE_SUB;
            break;
        }else if(token.type==TOKEN_MUL_EQUAL){
            ht=HANDLE_MUL;
            break;
        }else if(token.type==TOKEN_DIV_EQUAL){
            ht=HANDLE_DIV;
            break;
        }else if(token.type==TOKEN_AND_EQUAL){
            ht=HANDLE_AND;
            break;
        }else if(token.type==TOKEN_OR_EQUAL){
            ht=HANDLE_OR;
            break;
        }else if(token.type==TOKEN_LEFT_EQUAL){
            ht=HANDLE_LEFT;
            break;
        }else if(token.type==TOKEN_RIGHT_EQUAL){
            ht=HANDLE_RIGHT;
            break;
        }else{
            parser->ptr=rptr;
            parser->line=rline;
            return false;
        }
    }
    int sCount=stackPtr+1;
    int i=0;
    while(1){
        if(stackPtr<0){
            reportError(parser,"too many assignment.");
        }
        var=varList.vals[i];
        if(var.dim){
            addCmd2(clist,HANDLE_POPT,DATA_REG,DATA_INTEGER,REG_AX,stackPtr);
            if(!getArray(parser,clist,var,envirn)){
                reportError(parser,"expected an array in assignment.");
            }
        }else{
            if(!getExpression(parser,clist,&rtype,envirn)){
                reportError(parser,"expected an expression in assignment.");
            }
            addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_REG,REG_BX,REG_AX);
            addCmd2(clist,HANDLE_POPT,DATA_REG,DATA_INTEGER,REG_AX,stackPtr);
            if((var.class==TYPE_FLOAT && rtype.class==TYPE_INTEGER) || (var.class==TYPE_INTEGER && rtype.class==TYPE_INTEGER) || (var.class==TYPE_FLOAT && rtype.class==TYPE_FLOAT)){
                addCmd2(clist,ht,DATA_REG_POINTER,DATA_REG,REG_AX,REG_BX);
            }else if(var.class==TYPE_INTEGER && rtype.class==TYPE_FLOAT){
                addCmd2(clist,HANDLE_FTOI,DATA_REG,DATA_REG,REG_BX,REG_BX);
                addCmd2(clist,ht,DATA_REG_POINTER,DATA_REG,REG_AX,REG_BX);
            }else if(var.class!=rtype.class){
                sprintf(msg,"incompatible types when assigning to type \"%s\" from type \"%s\".",parser->classList.vals[var.class].name,parser->classList.vals[rtype.class].name);
                reportError(parser,msg);
            }else{
                /*对象赋值 */
                if(ht!=HANDLE_MOV){
                    sprintf(msg,"the type \"%s\" only support \"=\" to assign.",parser->classList.vals[var.class].name);
                    reportError(parser,msg);
                }
                addCmd2(clist,HANDLE_MOV,DATA_REG_POINTER,DATA_REG,REG_AX,REG_BX);
            }
        }
        stackPtr--;
        token=nextToken(parser);
        if(token.type==TOKEN_COMMA){
            continue;
            i++;
        }else if(token.type==TOKEN_SEMI){
            while(stackPtr>=0){
                var=varList.vals[i];
                if(var.dim){
                    reportError(parser,"too many array in assignment.");
                }else{
                    addCmd2(clist,HANDLE_POPT,DATA_REG,DATA_INTEGER,REG_AX,stackPtr);
                    if((var.class==TYPE_FLOAT && rtype.class==TYPE_INTEGER) || (var.class==TYPE_INTEGER && rtype.class==TYPE_INTEGER) || (var.class==TYPE_FLOAT && rtype.class==TYPE_FLOAT)){
                        addCmd2(clist,ht,DATA_REG_POINTER,DATA_REG,REG_AX,REG_BX);
                    }else if(var.class==TYPE_INTEGER && rtype.class==TYPE_FLOAT){
                        addCmd2(clist,HANDLE_FTOI,DATA_REG,DATA_REG,REG_BX,REG_BX);
                        addCmd2(clist,ht,DATA_REG_POINTER,DATA_REG,REG_AX,REG_BX);
                    }else if(var.class!=rtype.class){
                        sprintf(msg,"incompatible types when assigning to type \"%s\" from type \"%s\".",parser->classList.vals[var.class].name,parser->classList.vals[rtype.class].name);
                        reportError(parser,msg);
                    }else{
                        /*对象赋值 */
                        if(ht!=HANDLE_MOV){
                            sprintf(msg,"the type \"%s\" only support \"=\" to assign.",parser->classList.vals[var.class].name);
                            reportError(parser,msg);
                        }
                        addCmd2(clist,HANDLE_MOV,DATA_REG_POINTER,DATA_REG,REG_AX,REG_BX);
                    }
                }
                stackPtr--;
            }
            break;
        }else{
            reportError(parser,"expected \",\" or \";\" after the assignment.");
        }
    }
    addCmd1(clist,HANDLE_SFREE,DATA_INTEGER,sCount);
    LIST_DELETE(varList);
    return true;
}
bool getVarRef(Parser*parser,char*varName,CmdList*clist,Variable*var,Environment envirn){
    bool isFound=false;
    if(envirn.pvList!=NULL){
        for(int i=0;i<envirn.pvlist->count;i++){
            if(strcmp(varName,envirn.pvlist->vals[i].name)==0){
                *var=envirn.pvlist->vals[i];
                addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_INTEGER,REG_AX,envirn.pvlist->vals[i].ptr);
                isFound=true;
                break;
            }
        }
    }
    for(int i=0;i<parser->varlist.count && !isFound;i++){
        if(strcmp(varName,parser->varlist.vals[i].name)==0){
            *var=parser->varlist.vals[i];
            addCmd2(clist,HANDLE_MOV,DATA_REG,DATA_INTEGER,REG_AX,parser->varlist.vals[i].ptr);
            addCmd2(clist,HANDLE_ADD,DATA_REG,DATA_REG,REG_AX,REG_SP);
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
    ReturnType rtype;
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
            
        }else if(getInsideSub(parser,&parser->exeClist,envirn)){

        }else if(getExpression(parser,clist,&rtype,envirn)){
            token=nextToken(parser);
            if(token.type!=TOKEN_SEMI){
                reportError(parser,"expected \";\" after an expression");
            }
            //addCmd1(clist,HANDLE_SFREE,DATA_INTEGER,1);
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
    ReturnType rtype;
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
    if(!getExpression(parser,clist,&rtype,envirn)){
        reportError(parser,"expected an expression in the conditional statement.");
    }
    token=nextToken(parser);
    if(token.type!=TOKEN_PARE2){
        reportError(parser,"expected \")\" after \"if(expression...\".");
    }
    //addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
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
        if(!getExpression(parser,clist,&rtype,envirn)){
            reportError(parser,"expected an expression in the conditional statement.");
        }
        token=nextToken(parser);
        if(token.type!=TOKEN_PARE2){
            reportError(parser,"expected \")\" after \"if\".");
        }
        //addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
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
    ReturnType rtype;
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
    if(!getExpression(parser,clist,&rtype,envirn)){
        reportError(parser,"expected an expression in the loop statement.");
    }
    token=nextToken(parser);
    if(token.type!=TOKEN_PARE2){
        reportError(parser,"expected \")\" after \"while\".");
    }
    intList breakList;
    LIST_INIT(breakList,int);
    envirn.breakList=&breakList;
    //addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
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
bool getInsideSub(Parser*parser,CmdList*clist,Environment envirn){
    Token token;
    int rline,rptr;
    ReturnType rtype;
    rptr=parser->ptr;
    rline=parser->line;
    token=nextToken(parser);
    if(token.type==TOKEN_PUTC){
        token=nextToken(parser);
        if(token.type!=TOKEN_PARE1){
            reportError(parser,"expected \"(\" after putc.");
        }
        if(!getExpression(parser,clist,&rtype,envirn)){
            reportError(parser,"expected an expression in putc.");
        }
        token=nextToken(parser);
        if(token.type!=TOKEN_PARE2){
            reportError(parser,"expected \")\" after putc.");
        }
        token=nextToken(parser);
        if(token.type!=TOKEN_SEMI){
            reportError(parser,"expected \";\" after putc.");
        }
        if(rtype.class!=TYPE_INTEGER){
            reportError(parser,"putc only can output integer.");
        }
        //addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
        addCmd1(clist,HANDLE_PUTC,DATA_REG,REG_AX);
    }else{
        parser->ptr=rptr;
        parser->line=rline;
        return false;
    }
    return true;
}
bool getArray(Parser*parser,CmdList*clist,Variable var,Environment envirn){
    Token token;
    int rptr,rline;
    ReturnType rtype;
    char msg[100];
    int count=0;
    rptr=parser->ptr;
    rline=parser->line;
    token=nextToken(parser);
    if(token.type!=TOKEN_BRACE1){
        parser->line=rline;
        parser->ptr=rptr;
        return false;
    }
    while(1){
        count++;
        if(count>var.arrayCount){
            reportError(parser,"too many arrays.");
        }
        if(var.dim-1>0){
            if(!getArray(parser,clist,*var.subVar,envirn)){
                reportError(parser,"expected an array in the array.");
            }
        }else{
            addCmd1(clist,HANDLE_PUSH,DATA_REG,REG_AX);
            if(!getExpression(parser,clist,&rtype,envirn)){
                reportError(parser,"expected an expression in the array.");
            }
            if(var.class==TYPE_INTEGER && rtype.class==TYPE_FLOAT){
                addCmd1(clist,HANDLE_FTOI,DATA_REG,REG_AX);
            }else if(!(var.class==TYPE_FLOAT && rtype.class==TYPE_INTEGER) && var.class!=rtype.class){
                sprintf(msg,"can not cover the type \"%s\" from \"%s\".",parser->classList.vals[var.class].name,parser->classList.vals[var.class].name);
                reportError(parser,msg);
            }
            if(var.class==TYPE_FLOAT || var.class==TYPE_INTEGER){
                addCmd1(clist,HANDLE_POP,DATA_REG,REG_BX);
                addCmd2(clist,HANDLE_MOV,DATA_REG_POINTER,DATA_REG,REG_BX,REG_AX);
                addCmd2(clist,HANDLE_ADD,DATA_REG,DATA_INTEGER,REG_BX,1);
                addCmd1(clist,HANDLE_PUSH,DATA_REG,REG_BX);
            }else{
                /*对象赋值*/
            }
            addCmd1(clist,HANDLE_POP,DATA_REG,REG_AX);
        }
        token=nextToken(parser);
        if(token.type==TOKEN_COMMA){
            continue;
        }else if(token.type==TOKEN_BRACE2){
            break;
        }else{
            reportError(parser,"expected \",\" or \"}\" in the defination of the array.");
        }
    }
    if(count<var.arrayCount){
        reportError(parser,"too few arrays.");
    }
    return true;
}