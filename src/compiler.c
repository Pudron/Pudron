/***
*   Copyright (c) 2020 Pudron
*
*   This file is part of Pudron.
*
*   Pudron is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   Pudron is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
**/

#include"compiler.h"
const int OPERAT_PREFIX_COUNT=3;
const Operat operatPrefix[]={
    {TOKEN_EXCL,OPCODE_NOT,130},
    {TOKEN_INVERT,OPCODE_INVERT,180},
    {TOKEN_SUB,OPCODE_SUBS,100}
};
const int OPERAT_INFIX_COUNT=17;
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
Operat compileExpression(Compiler*cp,Unit*unit,int level,bool isAssign,int msgStart,Env env);
void compileBlock(Compiler*cp,Unit*unit,Env env);
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
void compileMsg(char msgType,Compiler*cp,wchar_t*text,int msgStart,...){
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
    va_list valist;
    va_start(valist,msgStart);
    wchar_t temp[MAX_STRING];
    vswprintf(temp,MAX_STRING-1,text,valist);
    va_end(valist);
    msg.type=msgType;
    reportMsg(msg,L"%ls",temp);
}
Compiler newCompiler(Parser parser,char*path,PdSTD pstd){
    Compiler compiler;
    compiler.parser=parser;
    compiler.pstd=pstd;
    compiler.path=path;
    compiler.charsetCode="UTF-8";
    return compiler;
}
Module compileAll(char*fileName,char*path,PdSTD pstd){
    Module mod;
    char*n1=cutPath(fileName);
    mod.name=cutPostfix(n1);
    free(n1);
    Unit emptyUnit=newUnit();
    setModuleUnit(&mod,emptyUnit);
    Compiler cp=newCompiler(newParser(fileName),path,pstd);
    getAllToken(&cp.parser);
    Env env={NULL,NULL,-1,true};
    Unit unit=newUnit();
    unit.gvlist=hashCopy(pstd.hl);
    LIST_ADD(unit.mlist,Module,mod)
    compileBlock(&cp,&unit,env);
    unit.curPart=-1;
    Const con;
    con.type=CONST_INT;
    con.num=0;
    addCmd1(&unit,OPCODE_LOAD_CONST,addConst(&unit,con));
    addCmd(&unit,OPCODE_RETURN);
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
            if(wcscmp(ct.str,con.str)==0){
                return i;
            }
        }
    }
    LIST_ADD(unit->constList,Const,con);
    return unit->constList.count-1;
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
void gete(Compiler*cp,Unit*unit,bool isAssign,int msgStart,Env env){
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
        con.str=strtowstr(token.word,cp->charsetCode);
        if(con.str==NULL){
            compileMsg(MSG_ERROR,cp,L"%s.",msgStart,strerror(errno));
        }
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
        if(hashGet(&unit->gvlist,token.word,NULL,false)<0){
            /*if(isAssign){
                hashGet(&unit->lvlist,token.word,NULL,true);
            }else{
                if(hashGet(&unit->lvlist,token.word,NULL,false)<0){
                    compileMsg(MSG_ERROR,cp,"variable \"%s\" no found.",msgStart,token.word);
                }
            }*/
            /*可能是类成员，所以isAssign在这里暂时没用*/
            hashGet(&unit->lvlist,token.word,NULL,true);
        }
        addCmd1(unit,OPCODE_LOAD_VAR,addName(&unit->nlist,token.word));
        token=nextToken(&cp->parser);
        if(token.type==TOKEN_PARE1){
            int count=0;
            while(1){
                token=nextToken(&cp->parser);
                if(token.type==TOKEN_PARE2){
                    break;
                }
                lastToken(&cp->parser);
                compileExpression(cp,unit,0,false,msgStart,env);
                count++;
                token=nextToken(&cp->parser);
                if(token.type!=TOKEN_COMMA){
                    lastToken(&cp->parser);
                }
            }
            addCmd1(unit,OPCODE_CALL_FUNCTION,count);
        }else{
            lastToken(&cp->parser);
        }
    }else{
        compileMsg(MSG_ERROR,cp,L"expected an expression.",msgStart);
    }
    while(1){
        token=nextToken(&cp->parser);
        if(token.type==TOKEN_POINT){
            token=matchToken(&cp->parser,TOKEN_WORD,"a member or method name",msgStart);
            char*name=token.word;
            token=nextToken(&cp->parser);
            if(token.type==TOKEN_PARE1){
                /*method*/
                addCmd1(unit,OPCODE_LOAD_METHOD,addName(&unit->nlist,name));
                int count=0;
                token=nextToken(&cp->parser);
                while(token.type!=TOKEN_PARE2){
                    lastToken(&cp->parser);
                    compileExpression(cp,unit,0,false,msgStart,env);
                    count++;
                    token=nextToken(&cp->parser);
                    if(token.type==TOKEN_COMMA){
                        token=nextToken(&cp->parser);
                        continue;
                    }else if(token.type==TOKEN_PARE2){
                        break;
                    }else{
                        compileMsg(MSG_ERROR,cp,L"expected \")\" or \",\" in the reference of method.",msgStart);
                    }
                }
                addCmd1(unit,OPCODE_CALL_METHOD,count);
            }else{
                /*member*/
                lastToken(&cp->parser);
                addCmd1(unit,OPCODE_LOAD_MEMBER,addName(&unit->nlist,name));
            }
        }else if(token.type==TOKEN_BRACKET1){
            int argCount=0;
            while(1){
                compileExpression(cp,unit,0,false,msgStart,env);
                argCount++;
                token=nextToken(&cp->parser);
                if(token.type==TOKEN_COMMA){
                    continue;
                }else if(token.type==TOKEN_BRACKET2){
                    break;
                }else{
                    compileMsg(MSG_ERROR,cp,L"expected \",\" or \"]\" in subscript.",msgStart);
                }
            }
            addCmd1(unit,OPCODE_LOAD_SUBSCRIPT,argCount);
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
            isFound=true;
            break;
        }
    }
    if(!isFound){
        lastToken(&cp->parser);
        gete(cp,unit,isAssign,msgStart,env);
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
    int scount=0;
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
    switch(token.type){
        case TOKEN_EQUAL:
            opt=-1;
            break;
        case TOKEN_ADD_EQUAL:
            opt=OPCODE_ADD;
            break;
        case TOKEN_SUB_EQUAL:
            opt=OPCODE_SUB;
            break;
        case TOKEN_MUL_EQUAL:
            opt=OPCODE_MUL;
            break;
        case TOKEN_DIV_EQUAL:
            opt=OPCODE_DIV;
            break;
        case TOKEN_LEFT_EQUAL:
            opt=OPCODE_LEFT;
            break;
        case TOKEN_RIGHT_EQUAL:
            opt=OPCODE_RIGHT;
            break;
        case TOKEN_AND_EQUAL:
            opt=OPCODE_AND;
            break;
        case TOKEN_OR_EQUAL:
            opt=OPCODE_OR;
            break;
        case TOKEN_PERCENT_EQUAL:
            opt=OPCODE_REM;
            break;
        default:
            compileMsg(MSG_ERROR,cp,L"expected assignment operator or \";\".",msgStart);
            break;
    }
    if(scount>1){
        addCmd1(unit,OPCODE_INVERT_ORDER,scount);
    }
    int gcount=0;
    while(1){
        compileExpression(cp,unit,0,false,msgStart,env);
        gcount++;
        token=nextToken(&cp->parser);
        if(token.type==TOKEN_COMMA){
            addCmd1(unit,OPCODE_ASSIGN,opt);
            continue;
        }else if(token.type==TOKEN_SEMI){
            if(gcount<scount){
                addCmd1(unit,OPCODE_SET_ASSIGN_COUNT,scount-gcount+1);
            }
            addCmd1(unit,OPCODE_ASSIGN,opt);
            unit->plist.vals[pt].end=token.end;
            break;
        }else{
            compileMsg(MSG_ERROR,cp,L"expected \";\" or \",\" in assignment.",msgStart);
        }
    }
    if(gcount>scount){
        compileMsg(MSG_ERROR,cp,L"too many assignment.",msgStart);
    }
}
/*不允许重复*/
void addNameNoRepeat(Compiler*cp,NameList*nlist,char*name,char*message,int msgStart){
    for(int i=0;i<nlist->count;i++){
        if(strcmp(name,nlist->vals[i])==0){
            compileMsg(MSG_ERROR,cp,L"the %s \"%s\" has already existed.",msgStart,message,name);
        }
    }
    LIST_ADD((*nlist),Name,name)
}
void compileFunction(Compiler*cp,Unit*unit,bool isMethod,Env env){
    env.breakList=NULL;
    Func func;
    Token token;
    Unit funit=newUnit();
    LIST_ADD(funit.mlist,Module,unit->mlist.vals[0]);
    int msgStart=cp->parser.tokenList.vals[cp->parser.curToken].start;
    token=matchToken(&cp->parser,TOKEN_WORD,"function name",msgStart);
    func.name=token.word;
    func.argCount=0;
    if(isMethod){
        addNameNoRepeat(cp,&env.classDef->varList,func.name,"member",msgStart);
        hashGet(&env.classDef->memberList,func.name,NULL,true);
        LIST_ADD(funit.nlist,Name,"this")
        hashGet(&funit.lvlist,"this",NULL,true);
        func.argCount++;
    }else{
        hashGet(&unit->lvlist,token.word,NULL,true);
    }
    funit.gvlist=hashMerge(unit->gvlist,unit->lvlist);
    matchToken(&cp->parser,TOKEN_PARE1,"\"(\" in function definition",msgStart);
    token=nextToken(&cp->parser);
    if(token.type!=TOKEN_PARE2){
        bool needArg=false;
        lastToken(&cp->parser);
        while(token.type!=TOKEN_PARE2 || needArg){
            token=matchToken(&cp->parser,TOKEN_WORD,"argument in function definition",msgStart);
            addNameNoRepeat(cp,&funit.nlist,token.word,"argument",msgStart);
            hashGet(&funit.lvlist,token.word,NULL,true);
            func.argCount++;
            token=nextToken(&cp->parser);
            if(token.type==TOKEN_COMMA){
                needArg=true;
            }else{
                needArg=false;
            }
        }
    }
    hashGet(&funit.lvlist,"argv",NULL,true);
    func.exe=NULL;
    compileBlock(cp,&funit,env);
    Const con;
    con.type=CONST_INT;
    con.num=0;
    addCmd1(&funit,OPCODE_LOAD_CONST,addConst(&funit,con));
    addCmd(&funit,OPCODE_RETURN);
    setFuncUnit(&func,funit);
    free(funit.gvlist.slot);
    funit.gvlist.slot=NULL;
    if(!isMethod){
        addCmd1(unit,OPCODE_LOAD_VAR,addName(&unit->nlist,func.name));
    }
    con.type=CONST_FUNCTION;
    con.func=func;
    addCmd1(unit,OPCODE_LOAD_CONST,addConst(unit,con));
    if(!isMethod){
        addCmd1(unit,OPCODE_ASSIGN,-1);
    }
}
void compileClass(Compiler*cp,Unit*unit){
    Class class;
    Token token;
    Const con;
    Env env={&class,NULL,-1,false};
    Unit funit=newUnit();
    LIST_ADD(funit.mlist,Module,unit->mlist.vals[0])
    class.initFunc.exe=NULL;
    class.initFunc.name=NULL;
    class.initFunc.argCount=1;
    class.memberList=newHashList();
    LIST_INIT(class.parentList)
    LIST_INIT(class.varList)
    NameList exdList;
    LIST_INIT(exdList)
    int msgStart=cp->parser.tokenList.vals[cp->parser.curToken].start;
    token=matchToken(&cp->parser,TOKEN_WORD,"class name",msgStart);
    class.name=token.word;
    hashGet(&unit->lvlist,class.name,NULL,true);
    funit.gvlist=hashMerge(unit->gvlist,unit->lvlist);
    token=nextToken(&cp->parser);
    if(token.type==TOKEN_COLON){
        while(1){
            token=matchToken(&cp->parser,TOKEN_WORD,"class name when extending",msgStart);
            hashGet(&class.memberList,token.word,NULL,true);
            addNameNoRepeat(cp,&exdList,token.word,"parent class",msgStart);
            token=nextToken(&cp->parser);
            if(token.type!=TOKEN_COMMA){
                break;
            }
        }
    }
    if(token.type!=TOKEN_BRACE1){
        compileMsg(MSG_ERROR,cp,L"expected \"{\" in class definition.",msgStart);
    }
    hashGet(&funit.lvlist,"this",NULL,true);
    int pt;
    while(1){
        token=nextToken(&cp->parser);
        msgStart=token.start;
        if(token.type==TOKEN_BRACE2){
            break;
        }else if(token.type==TOKEN_WORD){
            pt=setPart(cp,&funit,msgStart);
            addName(&class.varList,token.word);
            hashGet(&class.memberList,token.word,NULL,true);
            token=nextToken(&cp->parser);
            if(token.type==TOKEN_EQUAL){
                compileExpression(cp,&funit,0,false,msgStart,env);
                token=nextToken(&cp->parser);
            }else{
                con.type=CONST_INT;
                con.num=0;
                addCmd1(&funit,OPCODE_LOAD_CONST,addConst(&funit,con));
            }
            funit.plist.vals[pt].end=token.end;
            if(token.type!=TOKEN_SEMI){
                compileMsg(MSG_ERROR,cp,L"expected \";\" after class member definition.",msgStart);
            }
        }else if(token.type==TOKEN_FUNC){
            compileFunction(cp,&funit,true,env);
        }else if(token.type==TOKEN_CLASS){
            compileClass(cp,&funit);
        }else{
            compileMsg(MSG_ERROR,cp,L"expected class member",msgStart);
        }
    }
    setFuncUnit(&class.initFunc,funit);
    con.type=CONST_CLASS;
    con.class=class;
    addCmd1(unit,OPCODE_LOAD_VAR,addName(&unit->nlist,class.name));
    addCmd1(unit,OPCODE_LOAD_CONST,addConst(unit,con));
    /*继承*/
    for(int i=0;i<exdList.count;i++){
        addCmd1(unit,OPCODE_CLASS_EXTEND,addName(&unit->nlist,exdList.vals[i]));
    }
    addCmd1(unit,OPCODE_ASSIGN,-1);
    LIST_DELETE(exdList)
    free(funit.gvlist.slot);
}
void compileIfState(Compiler*cp,Unit*unit,Env env){
    Token token;
    intList endList;
    LIST_INIT(endList)
    int msgStart=cp->parser.tokenList.vals[cp->parser.curToken].start;
    int pt=setPart(cp,unit,msgStart);
    matchToken(&cp->parser,TOKEN_PARE1,"\"(\" in if statement",msgStart);
    compileExpression(cp,unit,0,false,msgStart,env);
    matchToken(&cp->parser,TOKEN_PARE2,"\")\" in if statement",msgStart);
    addCmd1(unit,OPCODE_JUMP_IF_FALSE,0);
    int jptr=unit->clist.count-1;
    unit->plist.vals[pt].end=cp->parser.ptr;
    compileBlock(cp,unit,env);
    token=nextToken(&cp->parser);
    while(token.type==TOKEN_ELIF){
        msgStart=token.start;
        pt=setPart(cp,unit,msgStart);
        addCmd1(unit,OPCODE_JUMP,0);
        LIST_ADD(endList,int,unit->clist.count-1)
        unit->clist.vals[jptr]=unit->clist.count;
        matchToken(&cp->parser,TOKEN_PARE1,"\"(\" in elif statement",msgStart);
        compileExpression(cp,unit,0,false,msgStart,env);
        matchToken(&cp->parser,TOKEN_PARE2,"\")\" in elif statement",msgStart);
        unit->plist.vals[pt].end=cp->parser.ptr;
        addCmd1(unit,OPCODE_JUMP_IF_FALSE,0);
        jptr=unit->clist.count-1;
        compileBlock(cp,unit,env);
        token=nextToken(&cp->parser);
    }
    if(token.type==TOKEN_ELSE){
        addCmd1(unit,OPCODE_JUMP,0);
        LIST_ADD(endList,int,unit->clist.count-1)
        unit->clist.vals[jptr]=unit->clist.count;
        compileBlock(cp,unit,env);
    }else{
        lastToken(&cp->parser);
        unit->clist.vals[jptr]=unit->clist.count;
    }
    for(int i=0;i<endList.count;i++){
        unit->clist.vals[endList.vals[i]]=unit->clist.count;
    }
    LIST_DELETE(endList)
}
void compileWhileState(Compiler*cp,Unit*unit,Env env){
    int msgStart=cp->parser.tokenList.vals[cp->parser.curToken].start;
    int pt=setPart(cp,unit,msgStart);
    Token token;
    intList breakList;
    LIST_INIT(breakList)
    intList*orblist=env.breakList;
    env.breakList=&breakList;
    matchToken(&cp->parser,TOKEN_PARE1,"\"(\" in while statement",msgStart);
    int jto=unit->clist.count;
    env.jumpTo=jto;
    compileExpression(cp,unit,0,false,msgStart,env);
    matchToken(&cp->parser,TOKEN_PARE2,"\")\" in while statement",msgStart);
    addCmd1(unit,OPCODE_JUMP_IF_FALSE,0);
    int jptr=unit->clist.count-1;
    unit->plist.vals[pt].end=cp->parser.ptr;
    compileBlock(cp,unit,env);
    addCmd1(unit,OPCODE_JUMP,jto);
    unit->clist.vals[jptr]=unit->clist.count;
    token=nextToken(&cp->parser);
    if(token.type==TOKEN_ELSE){
        env.breakList=orblist;
        compileBlock(cp,unit,env);
    }else{
        lastToken(&cp->parser);
    }
    for(int i=0;i<breakList.count;i++){
        unit->clist.vals[breakList.vals[i]]=unit->clist.count;
    }
    LIST_DELETE(breakList)
}
void compileDoWhileState(Compiler*cp,Unit*unit,Env env){
    intList breakList;
    LIST_INIT(breakList)
    env.breakList=&breakList;
    int jto=unit->clist.count;
    env.jumpTo=jto;
    compileBlock(cp,unit,env);
    int msgStart=cp->parser.ptr;
    setPart(cp,unit,msgStart);
    matchToken(&cp->parser,TOKEN_WHILE,"while in do-while statement",msgStart);
    matchToken(&cp->parser,TOKEN_PARE1,"\"(\" in do-while statement",msgStart);
    compileExpression(cp,unit,0,false,msgStart,env);
    matchToken(&cp->parser,TOKEN_PARE2,"\")\" in do-while statement",msgStart);
    matchToken(&cp->parser,TOKEN_SEMI,"\";\" after do-while statement",msgStart);
    addCmd(unit,OPCODE_NOT);
    addCmd1(unit,OPCODE_JUMP_IF_FALSE,jto);
    for(int i=0;i<breakList.count;i++){
        unit->clist.vals[breakList.vals[i]]=unit->clist.count;
    }
    LIST_DELETE(breakList)
}
/*将索引变量强制加入局部变量中*/
void compileForState(Compiler*cp,Unit*unit,Env env){
    intList breakList,*orblist;
    orblist=env.breakList;
    LIST_INIT(breakList)
    env.breakList=&breakList;
    int msgStart=cp->parser.tokenList.vals[cp->parser.curToken].start;
    int pt=setPart(cp,unit,msgStart);
    Const con;
    Token token;
    matchToken(&cp->parser,TOKEN_PARE1,"\"(\" in for statement",msgStart);
    char*varName=(matchToken(&cp->parser,TOKEN_WORD,"for statement index variable",msgStart)).word;
    matchToken(&cp->parser,TOKEN_COMMA,"\",\" in for statement",msgStart);
    compileExpression(cp,unit,0,false,msgStart,env);
    token=matchToken(&cp->parser,TOKEN_PARE2,"\")\" in for statement",msgStart);
    unit->plist.vals[pt].end=token.end;
    con.type=CONST_INT;
    con.num=0;
    addCmd1(unit,OPCODE_LOAD_CONST,addConst(unit,con));/*as index*/
    int jto=unit->clist.count;
    env.jumpTo=jto;
    addCmd1(unit,OPCODE_GET_FOR_INDEX,addName(&unit->nlist,varName));
    hashGet(&unit->lvlist,varName,NULL,true);
    addCmd1(unit,OPCODE_JUMP_IF_FALSE,0);
    int jptr=unit->clist.count-1;
    compileBlock(cp,unit,env);
    addCmd1(unit,OPCODE_JUMP,jto);
    unit->clist.vals[jptr]=unit->clist.count;
    token=nextToken(&cp->parser);
    if(token.type==TOKEN_ELSE){
        env.breakList=orblist;
        compileBlock(cp,unit,env);
    }else{
        lastToken(&cp->parser);
    }
    for(int i=0;i<breakList.count;i++){
        unit->clist.vals[breakList.vals[i]]=unit->clist.count;
    }
    LIST_DELETE(breakList)
}
void compileTry(Compiler*cp,Unit*unit,Env env){
    Token token=cp->parser.tokenList.vals[cp->parser.curToken];
    int msgStart=token.start;
    int pt=setPart(cp,unit,msgStart);
    unit->plist.vals[pt].end=token.end;
    addCmd1(unit,OPCODE_BEGIN_TRY,0);
    int jp=unit->clist.count-1;
    compileBlock(cp,unit,env);
    unit->curPart=pt;
    addCmd1(unit,OPCODE_END_TRY,0);
    unit->clist.vals[jp]=unit->clist.count;
    jp=unit->clist.count-1;
    msgStart=cp->parser.ptr;
    token=matchToken(&cp->parser,TOKEN_CATCH,"catch statement",msgStart);
    token=matchToken(&cp->parser,TOKEN_PARE1,"\"(\" in catch statement",msgStart);
    token=matchToken(&cp->parser,TOKEN_WORD,"error variable",msgStart);
    hashGet(&unit->lvlist,token.word,NULL,true);
    addCmd1(unit,OPCODE_SET_CATCH,addName(&unit->nlist,token.word));
    token=matchToken(&cp->parser,TOKEN_PARE2,"\")\" in catch statement",msgStart);
    compileBlock(cp,unit,env);
    unit->clist.vals[jp]=unit->clist.count;
}
/*模块存在则返回true*/
bool checkModuleName(ModuleList mlist,char*name){
    Module mod;
    for(int i=0;i<mlist.count;i++){
        mod=mlist.vals[i];
        if(strcmp(mod.name,name)==0){
            return true;
        }
        if(checkModuleName(mod.moduleList,name)){
            return true;
        }
    }
    return false;
}
void compileBlock(Compiler*cp,Unit*unit,Env env){
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
                compileMsg(MSG_ERROR,cp,L"expect \"}\" after block.",msgStart);
            }
            break;
        }else if(token.type==TOKEN_BRACE2 && !isGlobal){
            break;
        }else if(token.type==TOKEN_IF){
            compileIfState(cp,unit,env);
        }else if(token.type==TOKEN_WHILE){
            compileWhileState(cp,unit,env);
        }else if(token.type==TOKEN_DO){
            compileDoWhileState(cp,unit,env);
        }else if(token.type==TOKEN_FOR){
            compileForState(cp,unit,env);
        }else if(token.type==TOKEN_BREAK){
            if(env.breakList==NULL){
                compileMsg(MSG_ERROR,cp,L"invalid break.",token.start);
            }
            int pt=setPart(cp,unit,token.start);
            token=matchToken(&cp->parser,TOKEN_SEMI,"\";\" after break",token.start);
            unit->plist.vals[pt].end=token.end;
            addCmd1(unit,OPCODE_JUMP,0);
            LIST_ADD((*env.breakList),int,unit->clist.count-1)
        }else if(token.type==TOKEN_CONTINUE){
            if(env.jumpTo<0){
                compileMsg(MSG_ERROR,cp,L"invalid continue.",token.start);
            }
            int pt=setPart(cp,unit,token.start);
            token=matchToken(&cp->parser,TOKEN_SEMI,"\";\" after break",token.start);
            unit->plist.vals[pt].end=token.end;
            addCmd1(unit,OPCODE_JUMP,env.jumpTo);
        }else if(token.type==TOKEN_RETURN){
            int pt=setPart(cp,unit,token.start);
            compileExpression(cp,unit,0,false,token.start,env);
            addCmd(unit,OPCODE_RETURN);
            token=matchToken(&cp->parser,TOKEN_SEMI,"\";\" after returning",token.start);
            unit->plist.vals[pt].end=token.end;
        }else if(token.type==TOKEN_FUNC){
            compileFunction(cp,unit,false,env);
        }else if(token.type==TOKEN_CLASS){
            compileClass(cp,unit);
        }else if(token.type==TOKEN_INCLUDE){
            char*fileName=NULL,*modName=NULL;
            token=nextToken(&cp->parser);
            if(token.type==TOKEN_WORD){
                modName=token.word;
                fileName=(char*)memManage(NULL,strlen(modName)+strlen(FILE_CODE_POSTFIX)+1);
                sprintf(fileName,"%s%s",modName,FILE_CODE_POSTFIX);
            }else if(token.type==TOKEN_STRING){
                fileName=token.word;
                char*temp=cutPath(fileName);
                modName=cutPostfix(temp);
                free(temp);
            }else{
                compileMsg(MSG_ERROR,cp,L"expected a file name to include.",msgStart);
            }
            matchToken(&cp->parser,TOKEN_SEMI,"\";\" after include.",msgStart);
            if(checkModuleName(unit->mlist,modName)){
                /*模块已存在*/
                free(fileName);
                free(modName);
            }else{
                addCmd1(unit,OPCODE_LOAD_MODULE,unit->mlist.count);
                LIST_ADD(unit->mlist,Module,compileAll(fileName,cp->path,cp->pstd));
                free(fileName);
            }
        }else if(token.type==TOKEN_IMPORT){
            token=nextToken(&cp->parser);
            char*fileName=NULL,*modName=NULL;
            if(token.type==TOKEN_STRING){
                fileName=token.word;
                char*temp=cutPath(fileName);
                modName=cutPostfix(temp);
                free(temp);
            }else if(token.type==TOKEN_WORD){
                modName=token.word;
                fileName=(char*)memManage(NULL,strlen(cp->path)+strlen(modName)+strlen(FILE_MODULE_POSTFIX)+6);
                sprintf(fileName,"%s/mod/%s%s",cp->path,modName,FILE_MODULE_POSTFIX);
            }else{
                free(modName);
                free(fileName);
                compileMsg(MSG_ERROR,cp,L"expected a file name to import.",msgStart);
            }
            matchToken(&cp->parser,TOKEN_SEMI,"\";\" after import",msgStart);
            if(checkModuleName(unit->mlist,modName)){
                /*模块已存在*/
                free(fileName);
                free(modName);
            }else{
                addCmd1(unit,OPCODE_LOAD_MODULE,unit->mlist.count);
                LIST_ADD(unit->mlist,Module,importModule(fileName))
                free(fileName);
            }
        }else if(token.type==TOKEN_TRY){
            compileTry(cp,unit,env);
        }else if(token.type==TOKEN_RIGHT){
            token=matchToken(&cp->parser,TOKEN_WORD,"a setting option",msgStart);
            if(strcmp(token.word,"charset")==0){
                token=matchToken(&cp->parser,TOKEN_WORD,"a charset",msgStart);
                cp->charsetCode=token.word;
            }else{
                compileMsg(MSG_ERROR,cp,L"unknown setting option \"%s\".",msgStart,token.word);
            }
            matchToken(&cp->parser,TOKEN_SEMI,"\";\" after setting",msgStart);
        }else{
            lastToken(&cp->parser);
            compileAssignment(cp,unit,env);
        }
    }
}
