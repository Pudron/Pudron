#include"core.h"
#include"vm.h"
void vmError(VM*vm,char*text){
    Msg msg;
    Part part=vm->part;
    msg.fileName=part.fileName;
    msg.code=part.code;
    msg.line=part.line;
    msg.column=part.column;
    msg.start=part.start;
    msg.end=part.end;
    strcpy(msg.text,text);
    reportMsg(msg);
}
Func newFunc(){
    Func func;
    func.exe=NULL;
    LIST_INIT(func.argList)
    Unit unit=newUnit(0);
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
Object*newObjectStd(VM*vm,int class){
    Class*classd=&vm->stdclass[class];
    Object*obj=NULL;
    obj=(Object*)malloc(sizeof(Object));
    obj->class=classd;
    obj->varCount=classd->varList.count;
    obj->objs=(obj->varCount>0)?((Object**)malloc(sizeof(Object*)*obj->varCount)):NULL;
    obj->refCount=1;
    if(class==CLASS_STRING){
        obj->str=(char*)memManage(obj->str,1);
        obj->str[0]='\0';
    }
    return obj;
}
void reduceRef(VM*vm,Object*obj){
    obj->refCount--;
    if(obj->refCount<=0){
        if(obj->class->subID>=0){
            obj->refCount=2;
            Object*rt=obj->objs[obj->class->subID];
            if(!compareClassStd(vm,rt,CLASS_FUNCTION)){
                vmError(vm,"expected destroy method.");
            }
            PUSH(obj);
            callFunction(vm,rt->func,1);
        }
        for(int i=0;i<obj->varCount;i++){
            reduceRef(vm,obj->objs[i]);
        }
        if(obj->varCount>0){
            free(obj->objs);
        }
        free(obj);
    }
}
Object*loadConst(VM*vm,Unit*unit,int index){
    Object*obj=NULL;
    char temp[50];
    Const con=unit->constList.vals[index];
    switch(con.type){
        case CONST_INT:
            obj=newObjectStd(vm,CLASS_INT);
            obj->num=con.num;
            break;
        case CONST_DOUBLE:
            obj=newObjectStd(vm,CLASS_DOUBLE);
            obj->numd=con.numd;
            break;
        case CONST_STRING:
            obj=newObjectStd(vm,CLASS_STRING);
            obj->str=(char*)malloc(strlen(con.str)+1);
            strcpy(obj->str,con.str);
            break;
        case CONST_FUNCTION:
            obj=newObjectStd(vm,CLASS_FUNCTION);
            obj->func=con.func;
            break;
        case CONST_CLASS:
            obj=newObjectStd(vm,CLASS_CLASS);
            obj->classd=&con.classd;
            break;
        default:
            sprintf(temp,"unknown constant type:%d.",con.type);
            vmError(vm,temp);
            break;
    }
    return obj;
}
FUNC_DEF(std_init){
    Object*obj;
    for(int i=0;i<unit->constList.count;i++){
        obj=loadConst(vm,unit,i);
        PUSH(obj);
    }
}
void addClassFunc(Class*class,char*name,void*exe,int optID){
    Func func=newFunc();
    func.exe=exe;
    Var var={false,name,hashString(name)};
    LIST_ADD(class->varList,Var,var)
    if(optID>=1){
        class->optID[optID-1]=class->varList.count-1;
    }else if(optID==ID_INIT){
        class->initID=class->varList.count-1;
    }else if(optID==ID_DESTROY){
        class->destroyID=class->varList.count-1;
    }else if(optID==ID_SUBSCRIPT){
        class->subID=class->varList.count-1;
    }
    Const con;
    con.type=CONST_FUNCTION;
    con.func=func;
    LIST_ADD(class->initFunc.constList,Const,con)
}
void addClassInt(Class*class,char*name,int num){
    Var var={false,name,hashString(name)};
    LIST_ADD(class->varList,Var,var)
    Const con;
    con.type=CONST_INT;
    con.num=num;
    LIST_ADD(class->initFunc.constList,Const,con)
}
void addSubObj(Object*parent,Object*child){
    parent->varCount++;
    parent->objs=(Object**)memManage(parent->objs,parent->varCount*sizeof(Object*));
    parent->objs[parent->varCount-1]=child;
}
FUNC_DEF(string_create){
    char temp[50];
    Object*obj,*this=ARG(0);
    for(int i=1;i<ARGC;i++){
        obj=ARG(i);
        if(compareClassStd(vm,obj,CLASS_INT)){
            sprintf(temp,"%d",obj->num);
            this->str=(char*)memManage(this->str,strlen(this->str)+strlen(temp)+1);
            strcat(this->str,temp);
        }else if(compareClassStd(vm,obj,CLASS_DOUBLE)){
            sprintf(temp,"%f",obj->numd);
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
}
FUNC_DEF(string_add){
    Object*this=ARG(0),*obj=ARG(1);
    Object*str=newObjectStd(vm,CLASS_STRING);
    str->str=(char*)memManage(str->str,strlen(this->str)+strlen(obj->str)+1);
    sprintf(str->str,"%s%s",this->str,obj->str);
    STRING_LENGTH(this)=strlen(this->str);
    PD_RETURN(str);
}
FUNC_DEF(list_create){
    Object*this=ARG(0);
    for(int i=0;i<ARGC;i++){
        addSubObj(this,ARG(i));
        LIST_COUNT(this)++;
    }
}
FUNC_DEF(list_add){
    Object*this=ARG(0);
    addSubObj(this,ARG(1));
    LIST_COUNT(this)++;
}
FUNC_DEF(list_subscript){
    Object*this=ARG(0),*obj=ARG(1);
    if(!compareClassStd(vm,obj,CLASS_INT)){
        vmError(vm,"expected int when calling List subscript.");
    }
    Object*rt=this->objs[LIST_VAR_COUNT+obj->num];
    rt->refCount++;
    reduceRef(vm,this);
    reduceRef(vm,obj);
    PD_RETURN(rt);
}
void makeSTD(VM*vm){
    Class class;
    /*int definition*/
    class=newClass("int");
    class.initFunc=newFunc();
    class.initFunc.exe=NULL;
    vm->stdclass[CLASS_INT]=class;
    /*double definition*/
    class=newClass("double");
    class.initFunc=newFunc();
    class.initFunc.exe=NULL;
    vm->stdclass[CLASS_DOUBLE]=class;
    /*class definition*/
    class=newClass("Class");
    class.initFunc=newFunc();
    class.initFunc.exe=NULL;
    vm->stdclass[CLASS_CLASS]=class;
    /*function definition*/
    class=newClass("Function");
    class.initFunc=newFunc();
    class.initFunc.exe=NULL;
    vm->stdclass[CLASS_FUNCTION]=class;
    /*string definition*/
    class=newClass("string");
    class.initFunc=newFunc();
    class.initFunc.exe=std_init;
    addClassInt(&class,"length",0);
    addClassFunc(&class,"string_create",string_create,ID_INIT);
    addClassFunc(&class,"add",string_add,OPCODE_ADD);
    vm->stdclass[CLASS_STRING]=class;
    /*List definition*/
    class=newClass("List");
    class.initFunc=newFunc();
    class.initFunc.exe=std_init;
    addClassInt(&class,"count",0);
    addClassFunc(&class,"add",list_add,-1);
    addClassFunc(&class,"list_subscript",list_subscript,ID_SUBSCRIPT);
    vm->stdclass[CLASS_LIST]=class;
}