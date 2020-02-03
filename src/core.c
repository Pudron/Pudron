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
Func newFunc(){
    Func func;
    func.exe=NULL;
    LIST_INIT(func.argList)
    Unit unit=newUnit();
    setFuncUnit(&func,unit);
    return func;
}
Class newClass(char*name){
    Class class;
    class.name=name;
    class.hashName=hashString(name);
    LIST_INIT(class.varList)
    memset(class.optID,-1,OPT_METHOD_COUNT+1);
    class.initID=-1;
    class.destroyID=-1;
    class.subID=-1;
    class.initFunc=newFunc();
    return class;
}
bool compareClassStd(VM*vm,Object*obj,int class){
    if(obj->class->hashName==vm->stdclass[class].hashName){
        return true;
    }
    return false;
}
void callInitFunc(VM*vm,Class*class,Object*obj){
    Unit unit=getFuncUnit(class->initFunc);
    unit.varStart=vm->stackCount;
    obj->refCount++;
    PUSH(obj);
    vm->stack[vm->stackCount-1].hashName=hashString("this");
    if(class->initFunc.exe!=NULL){
        class->initFunc.exe(vm,&unit);
    }else{
        execute(vm,&unit);
    }
    for(int i=obj->varCount-1;i>=0;i--){
        obj->objs[i]=POP();
    }
    if(unit.flist.count>0){
        popStack(vm,unit.flist.vals[0].varList.count);
    }
    popStack(vm,1);/*pop this*/
}
void callFunction(VM*vm,Func func,int argc){

}
bool callMethod(VM*vm,Object*this,char*name){
    Object*obj=loadMember(vm,this,name,false);
    if(obj==NULL){
        return false;
    }
    confirmObjectType(vm,obj,OBJECT_FUNCTION);

}
Object*newObjectStd(VM*vm,int class){
    Class*classd=&vm->stdclass[class];
    Object*obj=NULL;
    obj=(Object*)malloc(sizeof(Object));
    obj->class=classd;
    obj->varCount=classd->varList.count;
    obj->objs=(obj->varCount>0)?((Object**)malloc(sizeof(Object*)*obj->varCount)):NULL;
    obj->refCount=1;
    obj->isInit=true;
    obj->num=0;
    if(compareClassStd(vm,obj,CLASS_STRING)){
        obj->str=(char*)memManage(obj->str,1);
        obj->str[0]='\0';
    }
    callInitFunc(vm,classd,obj);
    return obj;
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
void setLocal(VM*vm,Unit*unit,char*name,Object*obj){
    int index=hashGet(&unit->lvlist,name,false);
    if(index<0){
        vmError(vm,"local variable \"%s\" no found.",name);
    }
    unit->lvlist.slot[index].obj=obj;
}
void freeHashList(VM*vm,Unit*unit,HashList*hl){
    Object*obj;
    for(int i=0;i<hl->capacity;i++){
        obj=hl->slot[i].obj;
        if(obj!=NULL){
            reduceRef(vm,unit,obj);
        }
    }
}
void reduceRef(VM*vm,Unit*unit,Object*obj){
    obj->refCount--;
    if(obj->refCount<=0){
        Object*dobj=loadMember(vm,obj,METHOD_NAME_DESTROY,false);
        if(dobj!=NULL){
            obj->refCount=2;
            confirmObjectType(vm,dobj,OBJECT_FUNCTION);
            Unit funit=getFuncUnit(dobj->func);
            funit.lvlist=hashCopy(funit.lvlist);
            funit.gvlist=hashMerge(unit->gvlist,unit->lvlist);
            setLocal(vm,unit,"this",obj);
            execute(vm,&funit);
            reduceRef(vm,unit,POP());
            freeHashList(vm,unit,&funit.lvlist);
            free(funit.gvlist.slot);
        }
        free(obj);
    }
}
Object*newObject(char type){
    Object*obj=NULL;
    obj=(Object*)memManage(obj,sizeof(Object));
    obj->type=type;
    obj->member=newHashList();
    obj->refCount=1;
    obj->isInit=true;
    LIST_INIT(obj->classNameList);
    return obj;
}
Object*newIntObject(int num){
    Object*obj=NULL;
    obj=newObject(OBJECT_INT);
    LIST_ADD(obj->classNameList,Name,"int");
    obj->num=num;
    return obj;
}
Object*newDoubleObject(double numd){
    Object*obj=NULL;
    obj=newObject(OBJECT_DOUBLE);
    LIST_ADD(obj->classNameList,Name,"double");
    obj->numd=numd;
    return obj;
}
Object*newClassObject(Class class){
    Object*obj=NULL;
    obj=newObject(OBJECT_CLASS);
    LIST_ADD(obj->classNameList,Name,"Class");
    obj->class=class;
    return obj;
}
Object*newFuncObject(Func func){
    Object*obj=NULL;
    obj=newObject(OBJECT_FUNCTION);
    LIST_ADD(obj->classNameList,Name,"Func");
    obj->func=func;
    return obj;
}
Func newFunc(char*name){
    Func func;
    Unit unit=newUnit();
    func.name=name;
    func.exe=NULL;
    func.argCount=0;
    setFuncUnit(&func,unit);
}
Class newClass(char*name){
    Class class;
    class.name=name;
    LIST_INIT(class.parentList)
    class.memberList=newHashList();
    LIST_INIT(class.varList)
    class.initFunc=newFunc();
}
Object*loadConst(VM*vm,Unit*unit,int index){
    Object*obj=NULL;
    char temp[50];
    Const con=unit->constList.vals[index];
    switch(con.type){
        case CONST_INT:
            obj=newIntObject(con.num);
            break;
        case CONST_DOUBLE:
            obj=newDoubleObject(con.numd);
            break;
        case CONST_STRING:
            obj=newObjectStd(vm,CLASS_STRING);
            obj->str=NULL;
            obj->str=(char*)memManage(obj->str,strlen(con.str)+1);
            strcpy(obj->str,con.str);
            break;
        case CONST_FUNCTION:
            obj=newFuncObject(con.func);
            break;
        case CONST_CLASS:
            obj=newClassObject(con.class);
            break;
        default:
            sprintf(temp,"unknown constant type:%d.",con.type);
            vmError(vm,temp);
            break;
    }
    return obj;
}
Object*loadVar(VM*vm,Unit*unit,char*name){
    Object*obj;
    int index=hashGet(&unit->lvlist,name,false);
    if(index<0){
        index=hashGet(&unit->gvlist,name,false);
        if(index<0){
            vmError(vm,"variable \"%s\" no found.",name);
        }
        if(unit->gvlist.slot[index].obj==NULL){
            unit->gvlist.slot[index].obj=newIntObject(0);
        }
        obj=unit->gvlist.slot[index].obj;
    }else{
        if(obj=unit->lvlist.slot[index].obj==NULL){
            obj=unit->lvlist.slot[index].obj=newIntObject(0);
        }
        obj=unit->lvlist.slot[index].obj;
    }
    obj->refCount++;
    return obj;
}
Object*loadMember(VM*vm,Object*this,char*name,bool confirm){
    int index=hashGet(&this->member,name,false);
    if(index<0){
        if(!confirm){
            return NULL;
        }
        vmError(vm,"member \"%s\" in class \"%s\" no found.",name,this->classNameList.vals[0]);
    }
    Object*obj=this->member.slot[index].obj;
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
    for(int i=1;i<ARGC;i++){
        obj=ARG(i);
        if(compareClassStd(vm,obj,CLASS_INT)){
            sprintf(temp,"%d",obj->num);
            this->str=(char*)memManage(this->str,strlen(this->str)+strlen(temp)+1);
            strcat(this->str,temp);
        }else if(compareClassStd(vm,obj,CLASS_DOUBLE)){
            sprintf(temp,"%lf",obj->numd);
            this->str=(char*)memManage(this->str,strlen(this->str)+strlen(temp)+1);
            strcat(this->str,temp);
        }else if(compareClassStd(vm,obj,CLASS_STRING)){
            this->str=(char*)memManage(this->str,strlen(this->str)+strlen(obj->str)+1);
            strcat(this->str,obj->str);
        }else{
            PD_ERROR("expected string,int or double when creating string.");
        }
    }
    STRING_LENGTH(this)=strlen(this->str);
FUNC_END()
FUNC_DEF(string_add)
    Object*this=ARG(0),*obj=ARG(1);
    Object*str=newObjectStd(vm,CLASS_STRING);
    str->str=(char*)memManage(str->str,strlen(this->str)+strlen(obj->str)+1);
    sprintf(str->str,"%s%s",this->str,obj->str);
    STRING_LENGTH(this)=strlen(this->str);
    PD_RETURN(str);
FUNC_END()
FUNC_DEF(string_subscript)
    Object*this=ARG(0),*obj=ARG(1);
    if(!compareClassStd(vm,obj,CLASS_INT)){
        vmError(vm,"expected int when calling string subscript.");
    }
    Object*sto=newObjectStd(vm,CLASS_INT);
    if(sto->num>strlen(this->str)){
        PD_ERROR("string overflow");
    }
    sto->num=this->str[obj->num];
    PD_RETURN(sto);
FUNC_END()
FUNC_DEF(list_create)
    Object*this=loadVar(vm,unit,"this");
    Object*argv=loadVar(vm,unit,"argv");
    Object*obj,*cnt=loadMember(vm,argv,"count",true);
    this->type=OBJECT_LIST;
    this->subObj=NULL;
    this->subObj=(Object**)memManage(this->subObj,cnt->num*sizeof(Object*));
    for(int i=0;i<cnt->num;i++){
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
FUNC_DEF(print_stack)
    Object*obj;
    for(int i=0;i<vm->stackCount;i++){
        printf("stack%d:",i);
        obj=vm->stack[i].obj;
        if(compareClassStd(vm,obj,CLASS_INT)){
            printf("int:%d\n",obj->num);
        }else if(compareClassStd(vm,obj,CLASS_DOUBLE)){
            printf("int:%lf\n",obj->numd);
        }else if(compareClassStd(vm,obj,CLASS_CLASS)){
            printf("class\n");
        }else if(compareClassStd(vm,obj,CLASS_FUNCTION)){
            printf("function\n");
        }else if(compareClassStd(vm,obj,CLASS_STRING)){
            printf("string:%s\n",obj->str);
        }else if(compareClassStd(vm,obj,CLASS_LIST)){
            printf("List:%d\n",obj->varCount-LIST_VAR_COUNT);
        }else{
            printf("others\n");
        }
    }
FUNC_END()
FUNC_DEF(mprint)
    Object*obj;
    for(int i=0;i<ARGC;i++){
        obj=ARG(i);
        if(compareClassStd(vm,obj,CLASS_INT)){
            printf("%d",obj->num);
        }else if(compareClassStd(vm,obj,CLASS_DOUBLE)){
            printf("%lf",obj->numd);
        }else if(compareClassStd(vm,obj,CLASS_STRING)){
            printf(obj->str);
        }else{
            vmError(vm,"unsupported print type.");
        }
    }
FUNC_END()

PdSTD makeSTD(){
    PdSTD pstd;
    Class class;
    pstd.hl=newHashList();
    class=newClass("int");
    pstd.stdClass[0]=class;
    hashGet(&pstd.hl,"int",true);
    class=newClass("double");
    pstd.stdClass[1]=class;
    hashGet(&pstd.hl,"double",true);
    class=newClass("Func");
    pstd.stdClass[2]=class;
    hashGet(&pstd.hl,"Func",true);
    class=newClass("Class");
    pstd.stdClass[3]=class;
    hashGet(&pstd.hl,"Class",true);
    class=newClass("list");
    addClassInt(&class,"count",0);
    addClassFunc(&class,METHOD_NAME_INIT,list_create,0);
    addClassFunc(&class,"add",list_add,1,"element");
    addClassFunc(&class,METHOD_NAME_SUBSCRIPT,list_subscript,1,"index");
}