#include"core.h"
#include"vm.h"
#include"pio.h"
#include<locale.h>
#include<iconv.h>
void doexit(VM*vm,Unit*unit){
    while(vm->stackCount>0){
        reduceRef(vm,vm->unit,POP());
    }
    freeHashList(vm,unit,&unit->lvlist);
    freeHashList(vm,unit,&unit->gvlist);
    for(int i=0;i<vm->dlist.count;i++){
        if(vm->dlist.vals[i].dllptr!=NULL){
            DLL_CLOSE(vm->dlist.vals[i].dllptr);
            LIST_DELETE(vm->dlist.vals[i].dflist)
        }
    }
    LIST_DELETE(vm->dlist)
}
void vmError(VM*vm,wchar_t*text,...){
    Msg msg;
    Part part=vm->part;
    msg.fileName=part.fileName;
    msg.code=part.code;
    msg.line=part.line;
    msg.column=part.column;
    msg.start=part.start;
    msg.end=part.end;
    msg.type=MSG_ERROR;
    va_list valist;
    va_start(valist,text);
    wchar_t temp[MAX_STRING];
    vswprintf(temp,MAX_STRING-1,text,valist);
    for(int i=0;i<vm->plist.count;i++){
        part=vm->plist.vals[i];
        wprintf(L"from:%s:%d:%d:\n",part.fileName,part.line,part.column);
    }
    va_end(valist);
    doexit(vm,vm->unit);
    reportMsg(msg,L"%ls",temp);
}
void confirmObjectType(VM*vm,Object*obj,char type){
    if(obj->type!=type){
        switch(type){
            case OBJECT_INT:
                vmError(vm,L"expected an int type object.");
                break;
            case OBJECT_DOUBLE:
                vmError(vm,L"expected a double type object.");
                break;
            case OBJECT_FUNCTION:
                vmError(vm,L"expected a Func type object.");
                break;
            case OBJECT_CLASS:
                vmError(vm,L"expected a Class type object.");
                break;
            case OBJECT_LIST:
                vmError(vm,L"expected a list type object.");
                break;
            case OBJECT_STRING:
                vmError(vm,L"expected a string type object.");
                break;
            default:
                vmError(vm,L"object type do not equal.");
                break;
        }
    }
}
void setHash(VM*vm,HashList*hl,char*name,Object*obj){
    if(hashGet(hl,name,obj,false)<0){
        vmError(vm,L"the variable \"%s\" no found.",name);
    }
}
void freeHashList(VM*vm,Unit*unit,HashList*hl){
    Object*obj;
    for(int i=0;i<hl->capacity;i++){
        obj=hl->slot[i].obj;
        if(obj!=NULL){
            reduceRef(vm,unit,obj);
        }
    }
    free(hl->slot);
}
/*
*...为参数,若为method,则第一个参数为this,当argc=-1时,则后面接ArgList,
*若argc=-2,则只传入this,执行opDestroy()方法,不创建argv,防止无限释放argv
*/
void callFunction(VM*vm,Unit*unit,Func func,int argc,...){
    Object*obj;
    Unit funit=getFuncUnit(func);
    funit.gvlist=hashMerge(unit->gvlist,unit->lvlist);
    funit.lvlist=hashCopy(func.lvlist);
    va_list valist;
    va_start(valist,argc);
    Object*argv;
    int count;
    if(argc==-1){
        ArgList argList=va_arg(valist,ArgList);
        argc=argList.count;
        argv=newListObject(vm,argc);
        for(int i=0;i<argc;i++){
            argv->subObj[i]=argList.vals[i];
            argv->subObj[i]->isInit=false;/*防止赋值时被释放*/
        }
        //LIST_DELETE(argList)
        setHash(vm,&funit.lvlist,"argv",argv);
        count=(argc>func.argCount)?func.argCount:argc;
    }else if(argc==-2){
        obj=va_arg(valist,Object*);/*this*/
        setHash(vm,&funit.lvlist,"this",obj);
        count=0;
    }else{
        argv=newListObject(vm,argc);
        for(int i=0;i<argc;i++){
            argv->subObj[i]=va_arg(valist,Object*);
            argv->subObj[i]->isInit=false;
        }
        setHash(vm,&funit.lvlist,"argv",argv);
        count=(argc>func.argCount)?func.argCount:argc;
    }
    va_end(valist);
    for(int i=0;i<count;i++){
        obj=argv->subObj[i];
        obj->refCount++;
        setHash(vm,&funit.lvlist,funit.nlist.vals[i],obj);
    }
    int stackCount=vm->stackCount;
    if(func.exe!=NULL){
        func.exe(vm,&funit,&func);
    }else{
        execute(vm,&funit);
    }
    obj=POP();
    while(vm->stackCount>stackCount){
        reduceRef(vm,unit,POP());
    }
    freeHashList(vm,unit,&funit.lvlist);
    free(funit.gvlist.slot);
    PUSH(obj);
}
/*只删除现有数据，保持指针有效*/
void delObj(VM*vm,Unit*unit,Object*obj){
    Object*dobj=loadMember(vm,obj,METHOD_NAME_DESTROY,false);
    if(dobj!=NULL){
        obj->refCount=2;
        confirmObjectType(vm,dobj,OBJECT_FUNCTION);
        callFunction(vm,unit,dobj->func,-2,obj);
        reduceRef(vm,unit,POP());
        reduceRef(vm,unit,dobj);
    }
    LIST_DELETE(obj->classNameList)
}
void reduceRef(VM*vm,Unit*unit,Object*obj){
    obj->refCount--;
    if(obj->refCount<=0){
        delObj(vm,unit,obj);
        free(obj);
    }
}
Object*newObject(char type){
    Object*obj=(Object*)memManage(NULL,sizeof(Object));
    obj->type=type;
    //obj->member=newHashList();
    obj->refCount=1;
    obj->isInit=true;
    LIST_INIT(obj->classNameList);
    return obj;
}
Object*newIntObject(int num){
    Object*obj=newObject(OBJECT_INT);
    LIST_ADD(obj->classNameList,Name,"int");
    obj->member=newHashList();
    obj->num=num;
    return obj;
}
Object*newDoubleObject(double numd){
    Object*obj=newObject(OBJECT_DOUBLE);
    LIST_ADD(obj->classNameList,Name,"double");
    obj->member=newHashList();
    obj->numd=numd;
    return obj;
}
Object*newClassObject(Class class){
    Object*obj=newObject(OBJECT_CLASS);
    LIST_ADD(obj->classNameList,Name,"Class");
    obj->member=newHashList();
    obj->class=class;
    return obj;
}
Object*newFuncObject(Func func){
    Object*obj=newObject(OBJECT_FUNCTION);
    LIST_ADD(obj->classNameList,Name,"Func");
    obj->member=newHashList();
    obj->func=func;
    return obj;
}
Object*newStringObject(VM*vm,wchar_t*wstr){
    Class class=vm->pstd.stdClass[OBJECT_STRING];
    Object*obj=newObject(OBJECT_STRING);
    LIST_ADD(obj->classNameList,Name,"string")
    obj->member=hashCopy(class.memberList);
    Unit unit=getFuncUnit(class.initFunc);
    for(int i=0;i<class.varList.count;i++){
        setHash(vm,&obj->member,class.varList.vals[i],loadConst(vm,&unit,i));
    }
    obj->str=wstr;
    Object*len=loadMember(vm,obj,"length",true);
    if(wstr==NULL){
        len->num=0;
    }else{
        len->num=wcslen(wstr);
    }
    len->refCount--;
    return obj;
}
Object*newListObject(VM*vm,int count){
    Class class=vm->pstd.stdClass[OBJECT_LIST];
    Object*obj=newObject(OBJECT_LIST);
    LIST_ADD(obj->classNameList,Name,"List")
    obj->subObj=(Object**)memManage(NULL,sizeof(Object*)*count);
    obj->member=hashCopy(class.memberList);
    Unit unit=getFuncUnit(class.initFunc);
    for(int i=0;i<class.varList.count;i++){
        setHash(vm,&obj->member,class.varList.vals[i],loadConst(vm,&unit,i));
    }
    Object*cnt=loadMember(vm,obj,"count",true);
    cnt->num=count;
    cnt->refCount--;
    return obj;
}
Object*newErrorObject(VM*vm,ErrorType id,char*msg){
    Class class=vm->pstd.stdClass[OBJECT_ERROR];
    Object*obj=newObject(OBJECT_ERROR);
    LIST_ADD(obj->classNameList,Name,"Error")
    obj->member=hashCopy(class.memberList);
    Unit unit=getFuncUnit(class.initFunc);
    for(int i=0;i<class.varList.count;i++){
        setHash(vm,&obj->member,class.varList.vals[i],loadConst(vm,&unit,i));
    }
    Object*oid=loadMember(vm,obj,"id",true);
    Object*omsg=loadMember(vm,obj,"message",true);
    Object*len=loadMember(vm,omsg,"length",true);
    oid->num=id;
    omsg->str=strtowstr(msg,"UTF-8");
    if(omsg->str==NULL){
        vmError(vm,L"%s",strerror(errno));
    }
    len->num=wcslen(omsg->str);
    len->refCount--;
    omsg->refCount--;
    oid->refCount--;
    return obj;
}
Func newFunc(char*name){
    Func func;
    Unit unit=newUnit();
    func.name=name;
    func.exe=NULL;
    func.argCount=0;
    hashGet(&unit.lvlist,"argv",NULL,true);
    setFuncUnit(&func,unit);
    return func;
}
Class newClass(char*name){
    Class class;
    class.name=name;
    LIST_INIT(class.parentList)
    class.memberList=newHashList();
    LIST_INIT(class.varList)
    class.initFunc=newFunc("initFunc");
    LIST_ADD(class.initFunc.nlist,Name,"this")
    class.initFunc.argCount=1;
    hashGet(&class.initFunc.lvlist,"this",NULL,true);
    return class;
}
Object*loadConst(VM*vm,Unit*unit,int index){
    Object*obj=NULL;
    Const con=unit->constList.vals[index];
    wchar_t*wstr=NULL;
    switch(con.type){
        case CONST_INT:
            obj=newIntObject(con.num);
            break;
        case CONST_DOUBLE:
            obj=newDoubleObject(con.numd);
            break;
        case CONST_STRING:
            wstr=(wchar_t*)memManage(NULL,(wcslen(con.str)+1)*sizeof(wchar_t));
            wcscpy(wstr,con.str);
            obj=newStringObject(vm,wstr);
            break;
        case CONST_FUNCTION:
            obj=newFuncObject(con.func);
            break;
        case CONST_CLASS:
            obj=newClassObject(con.class);
            break;
        default:
            vmError(vm,L"unknown constant type:%d.",con.type);
            break;
    }
    //obj->isInit=false;
    return obj;
}
Object*loadVar(VM*vm,Unit*unit,char*name){
    Object*obj=NULL;
    if(vm->this!=NULL){
        obj=loadMember(vm,vm->this,name,false);
    }
    if(obj==NULL){
        int index=hashGet(&unit->lvlist,name,NULL,false);
        if(index<0){
            index=hashGet(&unit->gvlist,name,NULL,false);
            if(index<0){
                vmError(vm,L"variable \"%s\" no found.",name);
            }
            if(unit->gvlist.slot[index].obj==NULL){
                unit->gvlist.slot[index].obj=newIntObject(0);
            }
            obj=unit->gvlist.slot[index].obj;
        }else{
            if(unit->lvlist.slot[index].obj==NULL){
                unit->lvlist.slot[index].obj=newIntObject(0);
            }
            obj=unit->lvlist.slot[index].obj;
        }
    }
    obj->refCount++;
    return obj;
}
Object*loadMember(VM*vm,Object*this,char*name,bool confirm){
    Object*obj;
    int index=hashGet(&this->member,name,NULL,false);
    if(index<0){
        Object*parent;
        for(int i=1;i<this->classNameList.count;i++){
            parent=this->member.slot[hashGet(&this->member,this->classNameList.vals[i],NULL,false)].obj;
            obj=loadMember(vm,parent,name,false);
            if(obj!=NULL){
                return obj;
            }
        }
        if(!confirm){
            return NULL;
        }
        vmError(vm,L"member \"%s\" in class \"%s\" no found.",name,this->classNameList.vals[0]);
    }
    obj=this->member.slot[index].obj;
    obj->refCount++;
    return obj;
}
FUNC_DEF(std_init)
    Object*obj;
    for(int i=0;i<unit->constList.count;i++){
        obj=loadConst(vm,unit,i);
        PUSH(obj);
    }
}
void addClassFunc(Class*class,char*name,void*exe,int argCount,...){
    Func func=newFunc(name);
    func.exe=exe;
    addName(&class->varList,name);
    hashGet(&class->memberList,name,NULL,true);
    va_list valist;
    va_start(valist,argCount);
    char*argName;
    func.argCount=1;
    LIST_ADD(func.nlist,Name,"this")
    hashGet(&func.lvlist,"this",NULL,true);
    for(int i=0;i<argCount;i++){
        argName=va_arg(valist,char*);
        LIST_ADD(func.nlist,Name,argName)
        hashGet(&func.lvlist,argName,NULL,true);
        func.argCount++;
    }
    va_end(valist);
    Const con;
    con.type=CONST_FUNCTION;
    con.func=func;
    LIST_ADD(class->initFunc.constList,Const,con)
}
void addClassInt(Class*class,char*name,int num){
    addName(&class->varList,name);
    hashGet(&class->memberList,name,NULL,true);
    Const con;
    con.type=CONST_INT;
    con.num=num;
    LIST_ADD(class->initFunc.constList,Const,con)
}
void addClassString(Class*class,char*name,char*str){
    addName(&class->varList,name);
    hashGet(&class->memberList,name,NULL,true);
    Const con;
    con.type=CONST_STRING;
    con.str=strtowstr(str,"UTF-8");
    if(con.str==NULL){
        /*暂时用不了vmError和strerror了,(T_T)*/
        con.str=L"<string error>";
    }
    LIST_ADD(class->initFunc.constList,Const,con)
}
Func makeFunc(char*name,void*exe,int argCount,...){
    Func func=newFunc(name);
    func.exe=exe;
    va_list valist;
    va_start(valist,argCount);
    char*argName;
    for(int i=0;i<argCount;i++){
        argName=va_arg(valist,char*);
        LIST_ADD(func.nlist,Name,argName)
        hashGet(&func.lvlist,argName,NULL,true);
        func.argCount++;
    }
    va_end(valist);
    return func;
}
void printObjectValue(VM*vm,Unit*unit,Object*obj){
    switch(obj->type){
        case OBJECT_INT:
            wprintf(L"%d",obj->num);
            break;
        case OBJECT_DOUBLE:
            wprintf(L"%lf",obj->numd);
            break;
        case OBJECT_STRING:
            wprintf(L"%ls",obj->str);/*不能直接只传递字符串,防止被格式化(%xxx)*/
            break;
        case OBJECT_CLASS:
            wprintf(L"<%s>",obj->classNameList.vals[0]);
            break;
        case OBJECT_FUNCTION:
            wprintf(L"<function>");
            break;
        case OBJECT_LIST:{
            Object*cnt=loadMember(vm,obj,"count",true);
            wprintf(L"{");
            for(int i=0;i<cnt->num;i++){
                printObjectValue(vm,unit,obj->subObj[i]);
                if(i!=cnt->num-1){
                    wprintf(L",");
                }
            }
            wprintf(L"}");
            reduceRef(vm,unit,cnt);
            break;
        }
        default:
            wprintf(L"<object>");
            break;
    }
}
FUNC_DEF(string_create)
    wchar_t temp[64];
    Object*obj,*this=loadVar(vm,unit,"this");
    Object*argv=loadVar(vm,unit,"argv");
    Object*cnt=loadMember(vm,argv,"count",true);
    Object*len=loadMember(vm,this,"length",true);
    this->type=OBJECT_STRING;
    this->str=(wchar_t*)memManage(NULL,sizeof(wchar_t));
    this->str[0]=L'\0';
    for(int i=1;i<cnt->num;i++){
        obj=argv->subObj[i];
        switch(obj->type){
            case OBJECT_INT:
                swprintf(temp,63,L"%d",obj->num);
                this->str=(wchar_t*)memManage(this->str,(wcslen(this->str)+wcslen(temp)+1)*sizeof(wchar_t));
                wcscat(this->str,temp);
                break;
            case OBJECT_DOUBLE:
                swprintf(temp,63,L"%lf",obj->numd);
                this->str=(wchar_t*)memManage(this->str,(wcslen(this->str)+wcslen(temp)+1)*sizeof(wchar_t));
                wcscat(this->str,temp);
                break;
            case OBJECT_STRING:
                this->str=(wchar_t*)memManage(this->str,(wcslen(this->str)+wcslen(obj->str)+1)*sizeof(wchar_t));
                wcscat(this->str,obj->str);
                break;
            default:
                vmError(vm,L"expected string,int or double when creating string.");
                break;
        }
    }
    len->num=wcslen(this->str);
    reduceRef(vm,unit,cnt);
    reduceRef(vm,unit,argv);
    reduceRef(vm,unit,len);
    reduceRef(vm,unit,this);
FUNC_END()
FUNC_DEF(string_add)
    Object*this=loadVar(vm,unit,"this"),*obj=loadVar(vm,unit,"element");
    Object*str=newStringObject(vm,NULL);
    wchar_t temp[64];
    size_t len;
    switch(obj->type){
        case OBJECT_INT:
            swprintf(temp,63,L"%d",obj->num);
            str->str=(wchar_t*)memManage(str->str,(wcslen(this->str)+wcslen(temp)+1)*sizeof(wchar_t));
            wcscpy(str->str,this->str);
            wcscat(str->str,temp);
            break;
        case OBJECT_DOUBLE:
            swprintf(temp,63,L"%lf",obj->numd);
            str->str=(wchar_t*)memManage(str->str,(wcslen(this->str)+wcslen(temp)+1)*sizeof(wchar_t));
            wcscpy(str->str,this->str);
            wcscat(str->str,temp);
            break;
        case OBJECT_STRING:
            len=wcslen(this->str)+wcslen(obj->str)+1;
            str->str=(wchar_t*)memManage(str->str,len*sizeof(wchar_t));
            swprintf(str->str,len,L"%ls%ls",wcslen(this->str)+wcslen(obj->str),this->str,obj->str);
            break;
        default:
            len=wcslen(this->str)+9;
            str->str=(wchar_t*)memManage(str->str,len*sizeof(wchar_t));
            swprintf(str->str,len,L"%ls<object>",wcslen(this->str)+8,this->str);
            break;
    }
    Object*olen=loadMember(vm,str,"length",true);
    olen->num=wcslen(str->str);
    reduceRef(vm,unit,olen);
    reduceRef(vm,unit,this);
    PUSH(str);
    return;
FUNC_END()
FUNC_DEF(string_subscript)
    Object*this=loadVar(vm,unit,"this"),*ind=loadVar(vm,unit,"index");
    Object*argv=loadVar(vm,unit,"argv");
    Object*argc=loadMember(vm,argv,"count",true);
    confirmObjectType(vm,ind,OBJECT_INT);
    Object*err;
    int wslen=wcslen(this->str);
    if(ind->num>wslen){
        err=newErrorObject(vm,ERR_INDEX,"string overflow.");
        PUSH(err);
        goto fnd;
    }
    wchar_t*wstr;
    Object*sto;
    int index=(ind->num>=0)?ind->num:wslen+ind->num+1;
    if(argc->num==2){/*包括this*/
        wstr=(wchar_t*)memManage(NULL,2*sizeof(wchar_t));
        wstr[0]=this->str[index];
        wstr[1]=L'\0';
        sto=newStringObject(vm,wstr);
        PUSH(sto);
    }else if(argc->num==3){
        Object*end=loadVar(vm,unit,"end");
        confirmObjectType(vm,end,OBJECT_INT);
        int idend=(end->num>=0)?end->num:wslen+end->num+1;
        if(end->num>wslen){
            err=newErrorObject(vm,ERR_INDEX,"string end overflow.");
            PUSH(err);
        }else{
            if(index<=idend){
                wstr=cutWideText(this->str,index,idend);
            }else{
                int tlen=index-idend+1;
                wstr=(wchar_t*)memManage(NULL,(tlen+1)*sizeof(wchar_t));
                for(int i=0;i<tlen;i++){
                    wstr[i]=this->str[index-i];
                }
                wstr[tlen]=L'\0';
            }
            sto=newStringObject(vm,wstr);
            PUSH(sto);
        }
        reduceRef(vm,unit,end);
    }else{
        err=newErrorObject(vm,ERR_ARGUMENT,"invalid string subscript argument count.");
        PUSH(err);
    }
    fnd:reduceRef(vm,unit,argc);
    reduceRef(vm,unit,argv);
    reduceRef(vm,unit,ind);
    reduceRef(vm,unit,this);
    return;
FUNC_END()
FUNC_DEF(string_destroy)
     Object*this=loadVar(vm,unit,"this");
     free(this->str);
     reduceRef(vm,unit,this);
FUNC_END()
FUNC_DEF(string_equal)
    Object*this=loadVar(vm,unit,"this");
    Object*st=loadVar(vm,unit,"str");
    Object*rt;
    if(st->type!=OBJECT_STRING){
        rt=newIntObject(false);
    }else if(wcscmp(this->str,st->str)==0){
        rt=newIntObject(true);
    }else{
        rt=newIntObject(false);
    }
    reduceRef(vm,unit,st);
    reduceRef(vm,unit,this);
    PUSH(rt);
    return;
FUNC_END()
FUNC_DEF(list_create)
    Object*this=loadVar(vm,unit,"this");
    Object*argv=loadVar(vm,unit,"argv");
    Object*cnt=loadMember(vm,argv,"count",true);
    Object*cnt2=loadMember(vm,this,"count",true);
    cnt2->num=cnt->num-1;
    this->type=OBJECT_LIST;
    this->subObj=NULL;
    this->subObj=(Object**)memManage(this->subObj,cnt2->num*sizeof(Object*));
    for(int i=1;i<cnt->num;i++){
        this->subObj[i-1]=argv->subObj[i];
        argv->subObj[i]->refCount++;
    }
    reduceRef(vm,unit,cnt);
    reduceRef(vm,unit,argv);
    reduceRef(vm,unit,cnt2);
    reduceRef(vm,unit,this);
FUNC_END()
FUNC_DEF(list_add)
    Object*this=loadVar(vm,unit,"this");
    Object*element=loadVar(vm,unit,"element");
    Object*cnt=loadMember(vm,this,"count",true);
    cnt->num++;
    this->subObj=(Object**)memManage(this->subObj,cnt->num*sizeof(Object*));
    this->subObj[cnt->num-1]=element;
    reduceRef(vm,unit,cnt);
    reduceRef(vm,unit,this);
FUNC_END()
FUNC_DEF(list_subscript)
    Object*this=loadVar(vm,unit,"this");
    Object*cnt=loadMember(vm,this,"count",true);
    Object*ind=loadVar(vm,unit,"index");
    Object*argv=loadVar(vm,unit,"argv");
    Object*argc=loadMember(vm,argv,"count",true);
    confirmObjectType(vm,ind,OBJECT_INT);
    int index=(ind->num>=0)?ind->num:cnt->num+ind->num;
    Object*rt;
    if(index>=cnt->num){
        rt=newErrorObject(vm,ERR_INDEX,"list overflow.");
    }else{
        if(argc->num==2){
            rt=this->subObj[index];
            rt->refCount++;
        }else if(argc->num==3){
            Object*end=loadVar(vm,unit,"end");
            confirmObjectType(vm,end,OBJECT_INT);
            int idend=(end->num>=0)?end->num:cnt->num+end->num;
            if(idend>=cnt->num){
                rt=newErrorObject(vm,ERR_INDEX,"list overflow.");
            }else{
                int count;
                if(index<=idend){
                    count=idend-index+1;
                    rt=newListObject(vm,count);
                    for(int i=0;i<count;i++){
                        rt->subObj[i]=this->subObj[index+i];
                        rt->subObj[i]->refCount++;
                    }
                }else{
                    count=index-idend+1;
                    rt=newListObject(vm,count);
                    for(int i=0;i<count;i++){
                        rt->subObj[i]=this->subObj[index-i];
                        rt->subObj[i]->refCount++;
                    }
                }
            }
        }else{
            rt=newErrorObject(vm,ERR_ARGUMENT,"invalid list subscript argument count.");
        }
    }
    reduceRef(vm,unit,argc);
    reduceRef(vm,unit,argv);
    reduceRef(vm,unit,ind);
    reduceRef(vm,unit,cnt);
    reduceRef(vm,unit,this);
    PUSH(rt);
    return;
FUNC_END()
FUNC_DEF(list_destroy)
    Object*this=loadVar(vm,unit,"this");
    Object*cnt=loadMember(vm,this,"count",true);
    for(int i=0;i<cnt->num;i++){
        reduceRef(vm,unit,this->subObj[i]);
    }
    free(this->subObj);
    reduceRef(vm,unit,cnt);
    reduceRef(vm,unit,this);
FUNC_END()
FUNC_DEF(mprint)
    Object*argv=loadVar(vm,unit,"argv");
    Object*cnt=loadMember(vm,argv,"count",true);
    for(int i=0;i<cnt->num;i++){
        printObjectValue(vm,unit,argv->subObj[i]);
    }
    reduceRef(vm,unit,cnt);
    reduceRef(vm,unit,argv);
FUNC_END()
FUNC_DEF(mprintln)
    mprint(vm,unit,func);
    wprintf(L"\n");
FUNC_END()
FUNC_DEF(range)
    Object*argv=loadVar(vm,unit,"argv");
    Object*cnt=loadMember(vm,argv,"count",true);
    Object*a,*b,*c;
    Object*list=NULL;
    int count;
    if(cnt->num==1){
        a=loadVar(vm,unit,"a");
        confirmObjectType(vm,a,OBJECT_INT);
        list=newListObject(vm,a->num);
        for(int i=0;i<a->num;i++){
            list->subObj[i]=newIntObject(i);
        }
        reduceRef(vm,unit,a);
    }else if(cnt->num==2){
        a=loadVar(vm,unit,"a");
        b=loadVar(vm,unit,"b");
        confirmObjectType(vm,a,OBJECT_INT);
        confirmObjectType(vm,b,OBJECT_INT);
        if(a->num>b->num){
            vmError(vm,L"the start index(%d) must be less than the end index(%d).",a->num,b->num);
        }
        count=b->num-a->num+1;
        list=newListObject(vm,count);
        for(int i=0;i<count;i++){
            list->subObj[i]=newIntObject(i+a->num);
        }
        reduceRef(vm,unit,a);
        reduceRef(vm,unit,b);
    }else if(cnt->num==3){
        a=loadVar(vm,unit,"a");
        b=loadVar(vm,unit,"b");
        c=loadVar(vm,unit,"c");
        confirmObjectType(vm,a,OBJECT_INT);
        confirmObjectType(vm,b,OBJECT_INT);
        confirmObjectType(vm,c,OBJECT_INT);
        if(a->num>b->num){
            vmError(vm,L"the start index(%d) must be less than the end index(%d).",a->num,b->num);
        }
        if(c->num<=0){
            vmError(vm,L"the step length must be positive integer but not %d.",c->num);
        }
        count=b->num-a->num+1;
        list=newListObject(vm,count);
        for(int i=0;i<count;i++){
            list->subObj[i]=newIntObject(a->num+i*c->num);
        }
        reduceRef(vm,unit,a);
        reduceRef(vm,unit,b);
        reduceRef(vm,unit,c);
    }else{
        vmError(vm,L"the number of the arguments must be 1,2 or 3 for range().");
    }
    reduceRef(vm,unit,cnt);
    reduceRef(vm,unit,argv);
    PUSH(list);
    return;
FUNC_END()
FUNC_DEF(minput)
    char temp[MAX_WORD_LENGTH+1];
    //scanf("%[^\n]",st->str);
    fgets(temp,MAX_WORD_LENGTH,stdin);
    size_t leng=strlen(temp);
    wchar_t*wstr=(wchar_t*)memManage(NULL,sizeof(wchar_t)*(leng+1));
    if(mbstowcs(wstr,temp,leng)<0){
        vmError(vm,L"invalid characters.");
    }
    Object*st=newStringObject(vm,wstr);
    PUSH(st);
    return;
FUNC_END()
FUNC_DEF(mexit)
    Object*id=loadVar(vm,unit,"id");
    confirmObjectType(vm,id,OBJECT_INT);
    int d=id->num;
    reduceRef(vm,unit,id);
    while(vm->stackCount>0){
        reduceRef(vm,unit,POP());
    }
    freeHashList(vm,unit,&unit->lvlist);
    freeHashList(vm,unit,&unit->gvlist);
    exit(d);
FUNC_END()
FUNC_DEF(compare_class)
    Object*obj=loadVar(vm,unit,"object");
    Object*type=loadVar(vm,unit,"type");
    confirmObjectType(vm,type,OBJECT_CLASS);
    Object*rt;
    if(strcmp(obj->classNameList.vals[0],type->class.name)==0){
        rt=newIntObject(true);
    }else{
        rt=newIntObject(false);
    }
    reduceRef(vm,unit,type);
    reduceRef(vm,unit,obj);
    PUSH(rt);
    return;
FUNC_END()
FUNC_DEF(dll_create)
    Object*this=loadVar(vm,unit,"this");
    Object*id=loadMember(vm,this,"id",true);
    Object*st=loadVar(vm,unit,"path");
    this->type=OBJECT_DLL;
    confirmObjectType(vm,st,OBJECT_STRING);
    char*fname=wstrtostr(st->str,"UTF-8");
    if(fname==NULL){
        vmError(vm,L"open dll:%s.",strerror(errno));
    }
    Dllptr dptr=DLL_OPEN(fname);
    if(dptr==NULL){
        char*path=(char*)memManage(NULL,strlen(vm->path)+strlen(fname)+6);
        sprintf(path,"%s/mod/%s",vm->path,fname);
        dptr=DLL_OPEN(path);
        free(path);
    }
    free(fname);
    if(dptr==NULL){
        #ifdef LINUX
            vmError(vm,L"DLL:%s.",dlerror());
        #else
            vmError(vm,L"can not open the dll file,error code:%ld.",GetLastError());
        #endif
    }
    Dllinfo dllinfo;
    dllinfo.dllptr=dptr;
    LIST_INIT(dllinfo.dflist)
    id->num=vm->dlist.count;
    LIST_ADD(vm->dlist,Dllinfo,dllinfo)
    reduceRef(vm,unit,st);
    reduceRef(vm,unit,id);
    reduceRef(vm,unit,this);
FUNC_END()
FUNC_DEF(dll_destroy)
    Object*this=loadVar(vm,unit,"this");
    Object*id=loadMember(vm,this,"id",true);
    DLL_CLOSE(vm->dlist.vals[id->num].dllptr);
    vm->dlist.vals[id->num].dllptr=NULL;
    LIST_DELETE(vm->dlist.vals[id->num].dflist)
    reduceRef(vm,unit,id);
    reduceRef(vm,unit,this);
FUNC_END()
FUNC_DEF(dll_func)
    Object*argv=loadVar(vm,unit,"argv");
    Object*cnt=loadMember(vm,argv,"count",true);
    Dllinfo di=vm->dlist.vals[func->dllID];
    if(di.dllptr==NULL){
        vmError(vm,L"the dll function has been closed");
    }
    DllFunc df=di.dflist.vals[func->dllFuncID];
    _PDat pdat;
    pdat.rval.type=PVAL_INT;
    pdat.rval.num=0;
    LIST_INIT(pdat.argList)
    pdat.err_id=-1;
    _PValue pval;
    Object*obj=NULL;
    for(int i=0;i<cnt->num;i++){
        obj=argv->subObj[i];
        switch(obj->type){
            case OBJECT_INT:
                pval.type=PVAL_INT;
                pval.num=obj->num;
                break;
            case OBJECT_DOUBLE:
                pval.type=PVAL_DOUBLE;
                pval.numd=obj->numd;
                break;
            case OBJECT_STRING:{
                pval.type=PVAL_STRING;
                pval.str=(wchar_t*)memManage(NULL,(wcslen(obj->str)+1)*sizeof(wchar_t));
                wcscpy(pval.str,obj->str);
                break;
            }
            default:
                vmError(vm,L"dll function only accept type int,double and string arguments.");
                break;
        }
        LIST_ADD(pdat.argList,_PValue,pval)
    }
    df(&pdat);
    LIST_DELETE(pdat.argList)
    switch(pdat.rval.type){
        case PVAL_INT:
            obj=newIntObject(pdat.rval.num);
            break;
        case PVAL_DOUBLE:
            obj=newDoubleObject(pdat.rval.numd);
            break;
        case PVAL_STRING:{
            obj=newStringObject(vm,pdat.rval.str);
            break;
        }
        case PVAL_ERROR:{
            char*msg=wstrtostr(pdat.rval.str,"UTF-8");
            if(msg==NULL){
                vmError(vm,L"dll return error:%s.",strerror(errno));
            }
            obj=newErrorObject(vm,pdat.err_id,msg);
            break;
        }
        default:
            vmError(vm,L"unknown dll function return type:%d.",pdat.rval.type);
            break;
    }
    PUSH(obj);
    return;
FUNC_END()
FUNC_DEF(dll_get_func)
    Object*this=loadVar(vm,unit,"this");
    Object*id=loadMember(vm,this,"id",true);
    Object*fname=loadVar(vm,unit,"funcName");
    confirmObjectType(vm,fname,OBJECT_STRING);
    Func dfunc=newFunc(NULL);
    dfunc.dllID=id->num;
    char*funcName=wstrtostr(fname->str,"UTF-8");
    if(funcName==NULL){
        vmError(vm,L"get dll function:%s.",strerror(errno));
    }
    DllFunc df=DLL_GET(vm->dlist.vals[id->num].dllptr,funcName);
    if(df==NULL){
        #ifdef LINUX
            vmError(vm,L"DLL:%s.",dlerror());
        #else
            vmError(vm,L"can not get the dll function,error code:%ld.",GetLastError());
        #endif
    }
    free(funcName);
    funcName=NULL;
    dfunc.dllFuncID=vm->dlist.vals[dfunc.dllID].dflist.count;
    LIST_ADD(vm->dlist.vals[dfunc.dllID].dflist,DllFunc,df)
    dfunc.exe=dll_func;
    PUSH(newFuncObject(dfunc));
    reduceRef(vm,unit,fname);
    reduceRef(vm,unit,id);
    reduceRef(vm,unit,this);
    return;
FUNC_END()
FUNC_DEF(error_create)
    Object*this=loadVar(vm,unit,"this");
    Object*id=loadMember(vm,this,"id",true);
    Object*msg=loadMember(vm,this,"message",true);
    Object*argMsg=loadVar(vm,unit,"err_message");/*参数名不能与成员名一样*/
    confirmObjectType(vm,argMsg,OBJECT_STRING);
    Object*argID=loadVar(vm,unit,"err_id");
    confirmObjectType(vm,argID,OBJECT_INT);
    delObj(vm,unit,id);
    delObj(vm,unit,msg);
    free(id);
    free(msg);
    this->type=OBJECT_ERROR;
    setHash(vm,&this->member,"id",argID);
    setHash(vm,&this->member,"message",argMsg);
    reduceRef(vm,unit,this);
FUNC_END()
FUNC_DEF(get_platform)
    Object*obj;
    #ifdef LINUX
        obj=newIntObject(0);
    #else
        obj=newIntObject(1);
    #endif
    PUSH(obj);
    return;
FUNC_END()
FUNC_DEF(read_text_file)
    Object*fname=loadVar(vm,unit,"file");
    confirmObjectType(vm,fname,OBJECT_STRING);
    char*fileName=wstrtostr(fname->str,"UTF-8");
    if(fileName==NULL){
        vmError(vm,L"readTextFile:%s.",strerror(errno));
    }
    char*str=readTextFile(fileName);
    free(fileName);
    if(str==NULL){
        Object*err=newErrorObject(vm,ERR_FILE,strerror(errno));
        PUSH(err);
        return;
    }
    wchar_t*wstr;
    Object*argv=loadVar(vm,unit,"argv");
    Object*len=loadMember(vm,argv,"count",true);
    if(len->num==2){
        Object*ch=loadVar(vm,unit,"charset");
        confirmObjectType(vm,ch,OBJECT_STRING);
        char*code=wstrtostr(ch->str,"UTF-8");
        wstr=strtowstr(str,code);
        free(code);
        reduceRef(vm,unit,ch);
    }else{
        wstr=strtowstr(str,"UTF-8");
    }
    if(wstr==NULL){
        vmError(vm,L"%s",strerror(errno));
    }
    Object*ret=newStringObject(vm,wstr);
    reduceRef(vm,unit,len);
    reduceRef(vm,unit,argv);
    reduceRef(vm,unit,fname);
    PUSH(ret);
    return;
FUNC_END()
FUNC_DEF(write_text_file)
    Object*fname=loadVar(vm,unit,"file");
    Object*st=loadVar(vm,unit,"text");
    confirmObjectType(vm,fname,OBJECT_STRING);
    confirmObjectType(vm,st,OBJECT_STRING);
    char*fileName=wstrtostr(fname->str,"UTF-8");
    if(fileName==NULL){
        vmError(vm,L"writeTextFile:%s.",strerror(errno));
    }
    char*str;
    Object*argv=loadVar(vm,unit,"argv");
    Object*len=loadMember(vm,argv,"count",true);
    if(len->num==3){
        Object*ch=loadVar(vm,unit,"charset");
        confirmObjectType(vm,ch,OBJECT_STRING);
        char*code=wstrtostr(ch->str,"UTF-8");
        str=wstrtostr(st->str,code);
        free(code);
        reduceRef(vm,unit,ch);
    }else{
        str=wstrtostr(st->str,"UTF-8");
    }
    if(str==NULL){
        vmError(vm,L"writeTextFile:%s.",strerror(errno));
    }
    if(!writeTextFile(fileName,str)){
        Object*err=newErrorObject(vm,ERR_FILE,strerror(errno));
        PUSH(err);
        return;
    }
    reduceRef(vm,unit,len);
    reduceRef(vm,unit,argv);
    reduceRef(vm,unit,st);
    reduceRef(vm,unit,fname);
FUNC_END()
/*暂时没什么用处
FUNC_DEF(change_charset)
    Object*st=loadVar(vm,unit,"text");
    Object*toc=loadVar(vm,unit,"toCode");
    Object*foc=loadVar(vm,unit,"fromCode");
    char*toCode=wstrtostr(toc->str);
    char*fromCode=wstrtostr(foc->str);
    char*fstr=wstrtostr(st->str);
    if(toCode==NULL || fromCode==NULL || fstr==NULL){
        vmError(vm,L"changeCharset:%s.",strerror(errno));
    }
    iconv_t cd=iconv_open(toCode,fromCode);
    if(cd==(iconv_t)-1){
        vmError(vm,L"changeCharset:%s.",strerror(errno));
    }
    size_t flen=strlen(fstr),dlen=flen*2;
    char*dstr=(char*)memManage(NULL,(dlen+1)*sizeof(char));
    char*dptr=dstr,*fptr=fstr;
    if(iconv(cd,&fptr,&flen,&dptr,&dlen)==(size_t)-1){
        vmError(vm,L"changeCharset:%s.",strerror(errno));
    }
    iconv_close(cd);
    wchar_t*wstr=strtowstr(dstr);
    Object*rt=newStringObject(vm,wstr);
    free(toCode);
    free(fromCode);
    free(fstr);
    free(dstr);
    reduceRef(vm,unit,foc);
    reduceRef(vm,unit,toc);
    reduceRef(vm,unit,st);
    PUSH(rt);
    return;
FUNC_END()*/
PdSTD makeSTD(){
    PdSTD pstd;
    Class class;
    pstd.hl=newHashList();
    class=newClass("int");
    pstd.stdClass[OBJECT_INT]=class;
    hashGet(&pstd.hl,"int",NULL,true);

    class=newClass("double");
    pstd.stdClass[OBJECT_DOUBLE]=class;
    hashGet(&pstd.hl,"double",NULL,true);

    class=newClass("Func");
    pstd.stdClass[OBJECT_FUNCTION]=class;
    hashGet(&pstd.hl,"Func",NULL,true);

    class=newClass("Class");
    pstd.stdClass[OBJECT_CLASS]=class;
    hashGet(&pstd.hl,"Class",NULL,true);

    class=newClass("List");
    class.initFunc.exe=std_init;
    addClassInt(&class,"count",0);
    addClassFunc(&class,METHOD_NAME_INIT,list_create,0);
    addClassFunc(&class,"add",list_add,1,"element");
    addClassFunc(&class,METHOD_NAME_SUBSCRIPT,list_subscript,2,"index","end");
    addClassFunc(&class,METHOD_NAME_DESTROY,list_destroy,0);
    pstd.stdClass[OBJECT_LIST]=class;
    hashGet(&pstd.hl,"List",NULL,true);

    class=newClass("string");
    class.initFunc.exe=std_init;
    addClassInt(&class,"length",0);
    addClassFunc(&class,METHOD_NAME_INIT,string_create,0);
    addClassFunc(&class,METHOD_NAME_ADD,string_add,1,"element");
    addClassFunc(&class,METHOD_NAME_SUBSCRIPT,string_subscript,2,"index","end");
    addClassFunc(&class,METHOD_NAME_DESTROY,string_destroy,0);
    addClassFunc(&class,METHOD_NAME_EQUAL,string_equal,1,"str");
    pstd.stdClass[OBJECT_STRING]=class;
    hashGet(&pstd.hl,"string",NULL,true);

    class=newClass("DLL");
    class.initFunc.exe=std_init;
    addClassInt(&class,"id",-1);/*指向在vm->dllptrList中的位置*/
    addClassFunc(&class,METHOD_NAME_INIT,dll_create,1,"path");
    addClassFunc(&class,METHOD_NAME_DESTROY,dll_destroy,0);
    addClassFunc(&class,"getFunc",dll_get_func,1,"funcName");
    pstd.stdClass[OBJECT_DLL]=class;
    hashGet(&pstd.hl,"DLL",NULL,true);

    class=newClass("Error");
    class.initFunc.exe=std_init;
    addClassInt(&class,"id",0);
    addClassString(&class,"message","\0");
    addClassFunc(&class,METHOD_NAME_INIT,error_create,2,"err_id","err_message");
    pstd.stdClass[OBJECT_ERROR]=class;
    hashGet(&pstd.hl,"Error",NULL,true);

    Func func;
    func=makeFunc("print",mprint,0);
    pstd.stdFunc[0]=func;
    hashGet(&pstd.hl,"print",NULL,true);

    func=makeFunc("println",mprintln,0);
    pstd.stdFunc[1]=func;
    hashGet(&pstd.hl,"println",NULL,true);
    
    func=makeFunc("range",range,3,"a","b","c");
    pstd.stdFunc[2]=func;
    hashGet(&pstd.hl,"range",NULL,true);

    func=makeFunc("input",minput,0);
    pstd.stdFunc[3]=func;
    hashGet(&pstd.hl,"input",NULL,true);

    func=makeFunc("exit",mexit,1,"id");
    pstd.stdFunc[4]=func;
    hashGet(&pstd.hl,"exit",NULL,true);

    func=makeFunc("compareClass",compare_class,2,"object","type");
    pstd.stdFunc[5]=func;
    hashGet(&pstd.hl,"compareClass",NULL,true);

    func=makeFunc("getPlatform",get_platform,0);
    pstd.stdFunc[6]=func;
    hashGet(&pstd.hl,"getPlatform",NULL,true);

    func=makeFunc("readTextFile",read_text_file,2,"file","charset");
    pstd.stdFunc[7]=func;
    hashGet(&pstd.hl,"readTextFile",NULL,true);

    func=makeFunc("writeTextFile",write_text_file,3,"file","text","charset");
    pstd.stdFunc[8]=func;
    hashGet(&pstd.hl,"writeTextFile",NULL,true);

    return pstd;
}
void makeSTDObject(VM*vm,PdSTD*pstd){
    Object*obj;
    Class class;
    for(int i=0;i<STD_CLASS_COUNT;i++){
        class=pstd->stdClass[i];
        obj=newClassObject(class);
        setHash(vm,&pstd->hl,class.name,obj);
    }
    Func func;
    for(int i=0;i<STD_FUNC_COUNT;i++){
        func=pstd->stdFunc[i];
        obj=newFuncObject(func);
        setHash(vm,&pstd->hl,func.name,obj);
    }
}