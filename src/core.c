#include"core.h"
#include"vm.h"
void vmError(VM*vm,char*text,...){
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
    vsprintf(msg.text,text,valist);
    for(int i=0;i<vm->plist.count;i++){
        part=vm->plist.vals[i];
        printf("from:%s:%d:%d:\n",part.fileName,part.line,part.column);
    }
    va_end(valist);
    reportMsg(msg);
}
void confirmObjectType(VM*vm,Object*obj,char type){
    if(obj->type!=type){
        switch(type){
            case OBJECT_INT:
                vmError(vm,"expected an int type object.");
                break;
            case OBJECT_DOUBLE:
                vmError(vm,"expected a double type object.");
                break;
            case OBJECT_FUNCTION:
                vmError(vm,"expected a Func type object.");
                break;
            case OBJECT_CLASS:
                vmError(vm,"expected a Class type object.");
                break;
            case OBJECT_LIST:
                vmError(vm,"expected a list type object.");
                break;
            case OBJECT_STRING:
                vmError(vm,"expected a string type object.");
                break;
            default:
                vmError(vm,"object type do not equal.");
                break;
        }
    }
}
void setHash(VM*vm,HashList*hl,char*name,Object*obj){
    int index=hashGet(hl,name,false);
    if(index<0){
        vmError(vm,"variable \"%s\" no found.",name);
    }
    hl->slot[index].obj=obj;
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
/*...为参数,若为method,则第一个参数为this,当argc<0时,则后面接ArgList,ArgList会被DELETE*/
void callFunction(VM*vm,Unit*unit,Func func,int argc,...){
    Object*obj;
    Unit funit=getFuncUnit(func);
    funit.gvlist=hashMerge(unit->gvlist,unit->lvlist);
    funit.lvlist=hashCopy(func.lvlist);
    va_list valist;
    va_start(valist,argc);
    Object*argv=newListObject(vm);
    if(argc<0){
        ArgList argList=va_arg(valist,ArgList);
        argc=argList.count;
        for(int i=0;i<argc;i++){
            argv->subObj[i]=argList.vals[i];
        }
        LIST_DELETE(argList)
    }else{
        argv->subObj=(Object**)memManage(NULL,argc*sizeof(Object*));
        for(int i=0;i<argc;i++){
            argv->subObj[i]=va_arg(valist,Object*);
        }
    }
    va_end(valist);
    Object*cnt=loadMember(vm,argv,"count",true);
    cnt->num=argc;
    reduceRef(vm,unit,cnt);
    setHash(vm,&funit.lvlist,"argv",argv);
    int count=(argc>func.argCount)?func.argCount:argc;
    for(int i=0;i<count;i++){
        obj=argv->subObj[i];
        obj->refCount++;
        setHash(vm,&funit.lvlist,funit.nlist.vals[i],obj);
    }
    if(func.exe!=NULL){
        func.exe(vm,&funit);
    }else{
        execute(vm,&funit);
    }
    freeHashList(vm,unit,&funit.lvlist);
    free(funit.gvlist.slot);
}
/*只删除现有数据，保持指针有效*/
void delObj(VM*vm,Unit*unit,Object*obj){
    Object*dobj=loadMember(vm,obj,METHOD_NAME_DESTROY,false);
    if(dobj!=NULL){
        obj->refCount=2;
        confirmObjectType(vm,dobj,OBJECT_FUNCTION);
        callFunction(vm,unit,dobj->func,1,obj);
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
Object*newStringObject(VM*vm){
    Class class=vm->pstd.stdClass[OBJECT_STRING];
    Object*obj=newObject(OBJECT_STRING);
    LIST_ADD(obj->classNameList,Name,"string")
    obj->str=NULL;
    obj->member=hashCopy(class.memberList);
    Unit unit=getFuncUnit(class.initFunc);
    for(int i=0;i<class.varList.count;i++){
        setHash(vm,&obj->member,class.varList.vals[i],loadConst(vm,&unit,i));
    }
    return obj;
}
Object*newListObject(VM*vm){
    Class class=vm->pstd.stdClass[OBJECT_STRING];
    Object*obj=newObject(OBJECT_LIST);
    LIST_ADD(obj->classNameList,Name,"list")
    obj->subObj=NULL;
    obj->member=hashCopy(class.memberList);
    Unit unit=getFuncUnit(class.initFunc);
    for(int i=0;i<class.varList.count;i++){
        setHash(vm,&obj->member,class.varList.vals[i],loadConst(vm,&unit,i));
    }
    return obj;
}
Func newFunc(char*name){
    Func func;
    Unit unit=newUnit();
    func.name=name;
    func.exe=NULL;
    func.argCount=1;
    hashGet(&func.lvlist,"argv",true);
    setFuncUnit(&func,unit);
}
Class newClass(char*name){
    Class class;
    class.name=name;
    LIST_INIT(class.parentList)
    class.memberList=newHashList();
    LIST_INIT(class.varList)
    class.initFunc=newFunc("initFunc");
}
Object*loadConst(VM*vm,Unit*unit,int index){
    Object*obj=NULL;
    Const con=unit->constList.vals[index];
    switch(con.type){
        case CONST_INT:
            obj=newIntObject(con.num);
            break;
        case CONST_DOUBLE:
            obj=newDoubleObject(con.numd);
            break;
        case CONST_STRING:
            obj=newStringObject(vm);
            obj->str=(char*)memManage(NULL,strlen(con.str)+1);
            strcpy(obj->str,con.str);
            break;
        case CONST_FUNCTION:
            obj=newFuncObject(con.func);
            break;
        case CONST_CLASS:
            obj=newClassObject(con.class);
            break;
        default:
            vmError(vm,"unknown constant type:%d.",con.type);
            break;
    }
    return obj;
}
Object*loadVar(VM*vm,Unit*unit,char*name){
    Object*obj;
    if(vm->this!=NULL){
        obj=loadMember(vm,vm->this,name,false);
    }
    if(obj==NULL){
        int index=hashGet(&unit->gvlist,name,false);
        if(index<0){
            index-hashGet(&unit->lvlist,name,true);
            if(obj=unit->lvlist.slot[index].obj==NULL){
                obj=unit->lvlist.slot[index].obj=newIntObject(0);
            }
            obj=unit->lvlist.slot[index].obj;
        }else{
            if(unit->gvlist.slot[index].obj==NULL){
                unit->gvlist.slot[index].obj=newIntObject(0);
            }
            obj=unit->gvlist.slot[index].obj;
        }
    }
    obj->refCount++;
    return obj;
}
Object*loadMember(VM*vm,Object*this,char*name,bool confirm){
    Object*obj;
    int index=hashGet(&this->member,name,false);
    if(index<0){
        Object*parent;
        for(int i=1;i<this->classNameList.count;i++){
            parent=this->member.slot[hashGet(&this->member,this->classNameList.vals[i],false)].obj;
            obj=loadMember(vm,parent,name,false);
            if(obj!=NULL){
                return obj;
            }
        }
        if(!confirm){
            return NULL;
        }
        vmError(vm,"member \"%s\" in class \"%s\" no found.",name,this->classNameList.vals[0]);
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
    hashGet(&class->memberList,name,true);
    va_list valist;
    va_start(valist,argCount);
    char*argName;
    for(int i=0;i<argCount;i++){
        argName=va_arg(valist,char*);
        LIST_ADD(func.nlist,Name,argName)
        hashGet(&func.lvlist,argName,true);
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
    hashGet(&class->memberList,name,true);
    Const con;
    con.type=CONST_INT;
    con.num=num;
    LIST_ADD(class->initFunc.constList,Const,con)
}

FUNC_DEF(string_create)
    char temp[50];
    Object*obj,*this=loadVar(vm,unit,"this");
    Object*argv=loadVar(vm,unit,"argv");
    Object*cnt=loadMember(vm,argv,"count",true);
    Object*len=loadMember(vm,this,"length",true);
    for(int i=1;i<cnt->num;i++){
        obj=argv->subObj[i];
        switch(obj->type){
            case OBJECT_INT:
                sprintf(temp,"%d",obj->num);
                this->str=(char*)memManage(this->str,strlen(this->str)+strlen(temp)+1);
                strcat(this->str,temp);
                break;
            case OBJECT_DOUBLE:
                sprintf(temp,"%lf",obj->numd);
                this->str=(char*)memManage(this->str,strlen(this->str)+strlen(temp)+1);
                strcat(this->str,temp);
                break;
            case OBJECT_STRING:
                this->str=(char*)memManage(this->str,strlen(this->str)+strlen(obj->str)+1);
                strcat(this->str,obj->str);
                break;
            default:
                vmError(vm,"expected string,int or double when creating string.");
                break;
        }
    }
    len->num=strlen(this->str);
FUNC_END()
FUNC_DEF(string_add)
    Object*this=loadVar(vm,unit,"this"),*obj=loadVar(vm,unit,"element");
    Object*str=newStringObject(vm);
    str->str=(char*)memManage(str->str,strlen(this->str)+strlen(obj->str)+1);
    sprintf(str->str,"%s%s",this->str,obj->str);
    Object*len=loadMember(vm,str,"length",true);
    len->num=strlen(str->str);
    reduceRef(vm,unit,len);
    reduceRef(vm,unit,this);
    PUSH(str);
    return;
FUNC_END()
FUNC_DEF(string_subscript)
    Object*this=loadVar(vm,unit,"this"),*ind=loadVar(vm,unit,"index");
    confirmObjectType(vm,ind,OBJECT_INT);
    if(ind->num>strlen(this->str)){
        vmError(vm,"string overflow.");
    }
    Object*sto=newIntObject(this->str[ind->num]);
    PUSH(sto);
    return;
FUNC_END()
FUNC_DEF(string_destroy)
     Object*this=loadVar(vm,unit,"this");
     free(this->str);
     reduceRef(vm,unit,this);
FUNC_END()
FUNC_DEF(list_create)
    Object*this=loadVar(vm,unit,"this");
    Object*argv=loadVar(vm,unit,"argv");
    Object*obj,*cnt=loadMember(vm,argv,"count",true);
    this->type=OBJECT_LIST;
    this->subObj=NULL;
    this->subObj=(Object**)memManage(this->subObj,cnt->num*sizeof(Object*));
    for(int i=1;i<cnt->num;i++){
        this->subObj[i]=argv->subObj[i];
    }
    reduceRef(vm,unit,cnt);
    reduceRef(vm,unit,argv);
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
    Object*ind=loadVar(vm,unit,"index");
    confirmObjectType(vm,ind,OBJECT_INT);
    Object*rt=this->subObj[ind->num];
    rt->refCount++;
    reduceRef(vm,unit,ind);
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
    Object*obj;
    Object*argv=loadVar(vm,unit,"argv");
    Object*cnt=loadMember(vm,argv,"count",true);
    for(int i=0;i<cnt->num;i++){
        obj=argv->subObj[i];
        switch(obj->type){
            case OBJECT_INT:
                printf("%d",obj->num);
                break;
            case OBJECT_DOUBLE:
                printf("%lf",obj->numd);
                break;
            case OBJECT_STRING:
                printf("%s",obj->str);
                break;
            default:
                printf("<object>");
                break;
        }
    }
    reduceRef(vm,unit,cnt);
    reduceRef(vm,unit,argv);
FUNC_END()

PdSTD makeSTD(){
    PdSTD pstd;
    Class class;
    pstd.hl=newHashList();
    class=newClass("int");
    pstd.stdClass[OBJECT_INT]=class;
    hashGet(&pstd.hl,"int",true);
    class=newClass("double");
    pstd.stdClass[OBJECT_DOUBLE]=class;
    hashGet(&pstd.hl,"double",true);
    class=newClass("Func");
    pstd.stdClass[OBJECT_FUNCTION]=class;
    hashGet(&pstd.hl,"Func",true);
    class=newClass("Class");
    pstd.stdClass[OBJECT_CLASS]=class;
    hashGet(&pstd.hl,"Class",true);
    class=newClass("list");
    class.initFunc.exe=std_init;
    addClassInt(&class,"count",0);
    addClassFunc(&class,METHOD_NAME_INIT,list_create,0);
    addClassFunc(&class,"add",list_add,1,"element");
    addClassFunc(&class,METHOD_NAME_SUBSCRIPT,list_subscript,1,"index");
    addClassFunc(&class,METHOD_NAME_DESTROY,list_destroy,0);
    pstd.stdClass[OBJECT_LIST]=class;
    hashGet(&pstd.hl,"list",true);
    class=newClass("string");
    class.initFunc.exe=std_init;
    addClassInt(&class,"length",0);
    addClassFunc(&class,METHOD_NAME_ADD,string_add,1,"element");
    addClassFunc(&class,METHOD_NAME_SUBSCRIPT,string_subscript,1,"index");
    addClassFunc(&class,METHOD_NAME_DESTROY,string_destroy,0);
    pstd.stdClass[OBJECT_STRING]=class;
    return pstd;
}