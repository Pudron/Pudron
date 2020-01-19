#include"compiler.h"
#ifdef LINUX
#include<unistd.h>
#else
#include<windows.h>
#endif
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
//extern OpcodeMsg opcodeList[];
/*void initStd(Parser*parser){
    Class class;
    Part part;
    class.varBasis=0;
    class.var.count=0;
    class.methods.count=0;
    class.initID=-1;
    class.destroyID=-1;
    class.initValID=-1;
    for(int i=0;i<OPT_METHOD_COUNT;i++){
        class.optID[i]=-1;
    }
    LIST_INIT(class.parentList,int)
    LIST_ADD(class.parentList,int,CLASS_META)
    class.name=(char*)malloc(4);
    strcpy(class.name,"int");
    LIST_ADD(parser->classList,Class,class)
    class.name=(char*)malloc(5);
    strcpy(class.name,"class");
    LIST_ADD(parser->classList,Class,class)
    class.name=(char*)malloc(8);
    strcpy(class.name,"function");
    LIST_ADD(parser->classList,Class,class)
    part.fileName=parser->fileName;
    part.code=NULL;
    part.line=0;
    part.column=0;
    part.start=0;
    part.end=0;
    parser->curPart=parser->partList.count;
    LIST_ADD(parser->partList,Part,part)
    char temp[MAX_WORD_LENGTH];
    sprintf(temp,"%s/lib/meta.pdl",parser->path);
    import(parser,temp);
    sprintf(temp,"%s/lib/float.pdl",parser->path);
    import(parser,temp);
    sprintf(temp,"%s/lib/string.pdl",parser->path);
    import(parser,temp);
}
Parser compile(Parser*parent,char*fileName,bool isLib){
    Parser parser;
    initParser(&parser,(parent==NULL)?true:false);
    if(!readTextFile(&parser.code,fileName)){
        exit(-1);
    }
    parser.fileName=fileName;
    parser.isLib=isLib;
    if(parent==NULL){
        char*path=(char*)malloc(MAX_WORD_LENGTH);
        int len=-1;
        #ifdef LINUX
            len=readlink("/proc/self/exe",path,MAX_WORD_LENGTH-1);
            if(len<0){
                printf("error:failed to get pudron path.\n");
                exit(-1);
            }
            path[len]='\0';
        #else
            GetModuleFileName(NULL,path,MAX_WORD_LENGTH-1);
        #endif
        parser.path=getPath(path);
        free(path);
    }
    if(parent!=NULL){
        parser.partList=parent->partList;
        parser.clist=parent->clist;
        parser.symList=parent->symList;
        parser.funcList=parent->funcList;
        parser.classList=parent->classList;
        parser.moduleList=parent->moduleList;
        parser.path=parent->path;
    }else if(!isLib){
        initStd(&parser);
    }
    char*namet=cutPostfix(fileName);
    char*name2=cutPath(namet);
    free(namet);
    Module module={name2,0,0,0,0,0};
    parser.curModule=parser.moduleList.count;
    LIST_ADD(parser.moduleList,Module,module)
    Env env={-1,false,NULL,true,false};
    getAllToken(&parser);
    getBlock(&parser,&parser.clist,env);
    if(parent!=NULL){
        parent->partList=parser.partList;
        parent->clist=parser.clist;
        parent->symList=parser.symList;
        parent->funcList=parser.funcList;
        parent->classList=parser.classList;
    }
    return parser;
}
void run(char*fileName,bool isLib,char*outputName){
    Parser parser=compile(NULL,fileName,isLib);
    if(isLib){
        export(parser,outputName);
        puts("done\n");
    }else{
        VM vm;
        initVM(&vm,parser);
        execute(&vm,vm.clist);
        exitVM(&vm);
    }
}
void direct(char*fileName){
    Parser parser;
    initParser(&parser,true);
    parser.fileName=fileName;
    parser.isLib=false;
    initStd(&parser);
    Module module={fileName,0,0,0,0,0};
    parser.curModule=parser.moduleList.count;
    LIST_ADD(parser.moduleList,Module,module)
    import(&parser,fileName);
    VM vm;
    initVM(&vm,parser);
    execute(&vm,vm.clist);
}*/
void addCmd(Unit*unit,int opcode){
    LIST_ADD(unit->clist,int,unit->curPart);
    LIST_ADD(unit->clist,int,opcode);
}
void addCmd1(Unit*unit,int opcode,int dat){
    LIST_ADD(unit->clist,int,unit->curPart);
    LIST_ADD(unit->clist,int,opcode);
    LIST_ADD(unit->clist,int,dat);
}
void addCmds(Unit*unit,Command cmds){
    LIST_ADD(unit->clist,int,unit->curPart);
    for(int i=0;i<cmds.count;i++){
        LIST_ADD(unit->clist,int,cmds.code[i])
    }
}
void compileMsg(char msgType,Compiler*cp,char*text,int msgStart){
    Msg msg;
    msg.fileName=cp->parser.fileName;
    msg.code=cp->parser.code;
    msg.line=cp->parser.line;
    msg.column=cp->parser.column;
    if(msgStart<0){
        msg.end=0;
        msg.start=0;
    }else{
        msg.start=msgStart;
        msg.end=cp->parser.ptr;
    }
    strcpy(msg.text,text);
    msg.type=msgType;
    reportMsg(msg);
}
Compiler newCompiler(Parser parser){
    Compiler compiler;
    compiler.parser=parser;
    LIST_INIT(compiler.vlist)
    return compiler;
}
Module compileAll(char*fileName){
    Module mod;
    char*n1=cutPath(fileName);
    mod.name=cutPostfix(n1);
    free(n1);
    Compiler cp=newCompiler(newParser(fileName));
    getAllToken(&cp.parser);
    Env env={NULL,NULL,true,NULL};
    Unit unit=newUnit(0);
    compileBlock(&cp,&unit,env);
    addCmd(&unit,OPCODE_NOP);
    setModuleUnit(&mod,unit);
    return mod;
}
int addConst(Unit*unit,Const con){
    Const ct;
    for(int i=0;i<unit->constList.count;i++){
        ct=unit->constList.vals[i];
        if(ct.type==CONST_INT && con.type==CONST_INT){
            if(con.num==ct.num){
                return i;
            }
        }else if(ct.type==CONST_DOUBLE && con.type==CONST_DOUBLE){
            if(con.numd==ct.numd){
                return i;
            }
        }else if(ct.type==CONST_STRING && con.type==CONST_STRING){
            if(strcmp(ct.str,con.str)==0){
                return i;
            }
        }
    }
    LIST_ADD(unit->constList,Const,con);
    return unit->constList.count-1;
}
int getVar(Compiler*cp,Unit*unit,char*name,Env env){
    Var var;
    /*this module*/
    for(int i=cp->vlist.count-1;i>=unit->varStart;i--){
        var=cp->vlist.vals[i];
        if(strcmp(name,var.name)==0){
            return i-unit->varStart;
        }
    }
    /*upvalue in this module*/
    for(int i=unit->varStart-1;i>=0;i--){
        var=cp->vlist.vals[i];
        if(strcmp(name,var.name)==0){
            var.isRef=true;
            LIST_ADD(cp->vlist,Var,var)
            LIST_ADD(env.field->varList,Var,var)
            return cp->vlist.count-unit->varStart-1;
        }
    }
    /*add new*/
    var.name=name;
    var.isRef=false;
    var.hashName=hashString(name);
    LIST_ADD(cp->vlist,Var,var)
    LIST_ADD(env.field->varList,Var,var)
    return cp->vlist.count-unit->varStart-1;
}
int getMember(Unit*unit,char*name){
    for(int i=0;i<unit->mblist.count;i++){
        if(strcmp(unit->mblist.vals[i].name,name)==0){
            return i;
        }
    }
    Member mb;
    mb.name=name;
    mb.hashName=hashString(name);
    LIST_ADD(unit->mblist,Member,mb)
    return unit->mblist.count-1;
}
int setPart(Compiler*cp,Unit*unit,int start){
    Part part;
    part.code=cp->parser.code;
    part.fileName=cp->parser.fileName;
    part.line=cp->parser.line;
    part.column=cp->parser.column;
    part.start=start;
    part.end=start;
    unit->curPart=unit->plist.count;
    LIST_ADD(unit->plist,Part,part)
    return unit->curPart;
}
void gete(Compiler*cp,Unit*unit,int msgStart,Env env){
    Token token=nextToken(&cp->parser);
    if(token.type==TOKEN_INTEGER){
        Const con;
        con.type=CONST_INT;
        con.num=token.num;
        addCmd1(unit,OPCODE_LOAD_CONST,addConst(unit,con));
    }else if(token.type==TOKEN_DOUBLE){
        Const con;
        con.type=CONST_DOUBLE;
        con.numd=token.numd;
        addCmd1(unit,OPCODE_LOAD_CONST,addConst(unit,con));
    }else if(token.type==TOKEN_TRUE){
        Const con;
        con.type=CONST_INT;
        con.num=1;
        addCmd1(unit,OPCODE_LOAD_CONST,addConst(unit,con));
    }else if(token.type==TOKEN_FALSE){
        Const con;
        con.type=CONST_INT;
        con.num=0;
        addCmd1(unit,OPCODE_LOAD_CONST,addConst(unit,con));
    }else if(token.type==TOKEN_STRING){
        Const con;
        con.type=CONST_STRING;
        con.str=token.word;
        addCmd1(unit,OPCODE_LOAD_CONST,addConst(unit,con));
    }else if(token.type==TOKEN_PARE1){
        compileExpression(cp,unit,0,false,msgStart,env);
        matchToken(&cp->parser,TOKEN_PARE2,"\")\" in expression",msgStart);
    }else if(token.type==TOKEN_BRACE1){
        int count=0;
        bool needExp=false;
        token=nextToken(&cp->parser);
        while(token.type!=TOKEN_BRACE2 || needExp){
            lastToken(&cp->parser);
            compileExpression(cp,unit,0,false,msgStart,env);
            count++;
            token=nextToken(&cp->parser);
            if(token.type==TOKEN_COMMA){
                token=nextToken(&cp->parser);
                needExp=true;
            }else{
                needExp=false;
            }
        }
        addCmd1(unit,OPCODE_MAKE_ARRAY,count);
    }else if(token.type==TOKEN_WORD){
        int count=-1;
        char*name=token.word;
        token=nextToken(&cp->parser);
        if(token.type==TOKEN_PARE1){
            count=0;
            while(token.type!=TOKEN_PARE2){
                compileExpression(cp,unit,0,false,msgStart,env);
                count++;
                token=nextToken(&cp->parser);
                if(token.type==TOKEN_COMMA){
                    continue;
                }else if(token.type==TOKEN_PARE2){
                    break;
                }else{
                    compileMsg(MSG_ERROR,cp,"expected \")\" or \",\" in the reference of function.",msgStart);
                }
            }
        }else{
            lastToken(&cp->parser);
        }
        bool isFound=false;
        if(env.class!=NULL){
            for(int i=0;i<env.class->var.count;i++){
                if(strcmp(name,env.class->var.vals[i])){
                    isFound=true;
                    addCmd1(unit,OPCODE_LOAD_VAR,getVar(cp,unit,"this",env));
                    addCmd1(unit,OPCODE_LOAD_ATTR,i);
                    break;
                }
            }
        }
        if(!isFound){
            addCmd1(unit,OPCODE_LOAD_VAR,getVar(cp,unit,name,env));
        }
        if(count>=0){
            addCmd1(unit,OPCODE_CALL_FUNCTION,count);
        }
    }else{
        compileMsg(MSG_ERROR,cp,"expected an expression.",msgStart);
    }
    while(1){
        token=nextToken(&cp->parser);
        if(token.type==TOKEN_POINT){
            token=matchToken(&cp->parser,TOKEN_WORD,"a member or method name",msgStart);
            char*name=token.word;
            token=nextToken(&cp->parser);
            if(token.type==TOKEN_PARE1){
                /*method*/
                int count=0;
                while(token.type!=TOKEN_PARE2){
                    compileExpression(cp,unit,0,false,msgStart,env);
                    count++;
                    token=nextToken(&cp->parser);
                    if(token.type==TOKEN_COMMA){
                        continue;
                    }else if(token.type==TOKEN_PARE2){
                        break;
                    }else{
                        compileMsg(MSG_ERROR,cp,"expected \")\" or \",\" in the reference of method.",msgStart);
                    }
                }
                addCmd1(unit,OPCODE_STACK_COPY,count);
                addCmd1(unit,OPCODE_LOAD_MEMBER,getMember(unit,name));
                addCmd1(unit,OPCODE_CALL_FUNCTION,count+1);
            }else{
                /*member*/
                lastToken(&cp->parser);
                addCmd1(unit,OPCODE_LOAD_MEMBER,getMember(unit,name));
            }
        }else if(token.type==TOKEN_BRACKET1){
            compileExpression(cp,unit,0,false,msgStart,env);
            matchToken(&cp->parser,TOKEN_BRACKET2,"\"]\" in expression",msgStart);
            addCmd(unit,OPCODE_LOAD_SUBSCRIPT);
        }else{
            lastToken(&cp->parser);
            break;
        }
    }
}
Operat compileExpression(Compiler*cp,Unit*unit,int level,bool isAssign,int msgStart,Env env){
    Token token=nextToken(&cp->parser);
    Operat opt;
    bool isFound=false;
    for(int i=0;i<OPERAT_PREFIX_COUNT;i++){
        if(token.type==operatPrefix[i].tokenType){
            opt=operatPrefix[i];
            level=opt.level;
            isFound=true;
            break;
        }
    }
    if(!isFound){
        lastToken(&cp->parser);
        gete(cp,unit,msgStart,env);
        token=nextToken(&cp->parser);
        for(int i=0;i<OPERAT_INFIX_COUNT;i++){
            if(token.type==operatInfix[i].tokenType){
                opt=operatInfix[i];
                if(opt.tokenType!=TOKEN_EQUAL || !isAssign){
                    isFound=true;
                }
                break;
            }
        }
        if(!isFound){
            opt.level=0;
            lastToken(&cp->parser);
        }
    }
    int opcode;
    while(opt.level>level){
        opcode=opt.opcode;
        opt=compileExpression(cp,unit,opt.level,isAssign,msgStart,env);
        addCmd(unit,opcode);
    }
    return opt;
}
void compileAssignment(Compiler*cp,Unit*unit,Env env){
    Token token;
    int msgStart=cp->parser.tokenList.vals[cp->parser.curToken+1].start;
    int pt=setPart(cp,unit,msgStart);
    int scount=0,gcount=0;
    while(1){
        compileExpression(cp,unit,0,true,msgStart,env);
        scount++;
        token=nextToken(&cp->parser);
        if(token.type!=TOKEN_COMMA){
            break;
        }
    }
    if(token.type==TOKEN_SEMI){
        addCmd1(unit,OPCODE_POP_STACK,scount);
        unit->plist.vals[pt].end=token.end;
        return;
    }
    int opt=-1;
    for(int i=0;i<OPERAT_INFIX_COUNT;i++){
        if(operatInfix[i].tokenType==token.type && token.type!=TOKEN_EQUAL){
            opt=operatInfix[i].opcode;
            token=nextToken(&cp->parser);
            break;
        }
    }
    if(token.type!=TOKEN_EQUAL){
        compileMsg(MSG_ERROR,cp,"expected assignment operator.",msgStart);
    }
    addCmd1(unit,OPCODE_ASSIGN_LEFT,scount);
    while(1){
        compileExpression(cp,unit,0,false,msgStart,env);
        gcount++;
        token=nextToken(&cp->parser);
        if(token.type==TOKEN_COMMA){
            continue;
        }else if(token.type==TOKEN_SEMI){
            unit->plist.vals[pt].end=token.end;
            break;
        }else{
            compileMsg(MSG_ERROR,cp,"expected \";\" or \",\" in assignment.",msgStart);
        }
    }
    addCmd1(unit,OPCODE_ASSIGN_RIGHT,gcount);
    addCmd1(unit,OPCODE_ASSIGN,opt);
}
void compileBlock(Compiler*cp,Unit*unit,Env env){
    addCmd1(unit,OPCODE_LOAD_FIELD,0);
    int fptr=unit->clist.count-1;
    Field field;
    LIST_INIT(field.varList)
    env.field=&field;
    Token token;
    int msgStart=cp->parser.tokenList.vals[cp->parser.curToken].start;
    bool isGlobal=env.isGlobal;
    if(env.isGlobal){
        env.isGlobal=false;
    }else{
        matchToken(&cp->parser,TOKEN_BRACE1,"\"{\" in block",msgStart);
    }
    while(1){
        token=nextToken(&cp->parser);
        msgStart=token.start;
        if(token.type==TOKEN_END){
            if(!isGlobal){
                compileMsg(MSG_ERROR,cp,"expect \"}\" after block.",msgStart);
            }
            break;
        }else if(token.type==TOKEN_BRACE2 && !isGlobal){
            break;
        }else{
            lastToken(&cp->parser);
            compileAssignment(cp,unit,env);
        }
    }
    unit->clist.vals[fptr]=unit->flist.count;
    if(!isGlobal){
        addCmd1(unit,OPCODE_FREE_FIELD,unit->flist.count);
    }
    LIST_ADD(unit->flist,Field,field)
}