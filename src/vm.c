#include"vm.h"
#ifdef DEBUG
#define ASSERT(condition,message,...) \
    if(condition){\
        printf(message,##__VA_ARGS__);\
        getchar();\
    }
#else
#define ASSERT(condition,message,...)
#endif
#define checkStack(num) ASSERT(vm->stackCount<num,"%d:expected %d slots but only %d slots in stack.",vm->ptr,num,vm->stackCount)
VM newVM(char*fileName){
    VM vm;
    vm.stackCount=0;
    vm.ptr=0;
    vm.part.fileName=fileName;
    vm.part.code=NULL;
    vm.part.line=0;
    vm.part.column=0;
    vm.part.start=0;
    vm.part.end=0;
    LIST_INIT(vm.loopList)
    LIST_INIT(vm.plist)
    makeSTD(&vm);
    return vm;
}
void popStack(VM*vm,int num){
    for(int i=0;i<num;i++){
        reduceRef(vm,POP());
    }
}
void callFunction(VM*vm,Func func,int argc){
    Unit unit=getFuncUnit(func);
    unit.varStart=vm->stackCount-argc;
    unit.argc=argc;
    int loopc=vm->loopList.count;
    int nc;
    if(argc<func.argList.count){
        Object*obj=newObjectStd(vm,CLASS_INT);
        obj->num=0;
        nc=func.argList.count-argc;
        for(int i=0;i<nc;i++){
            PUSH(obj);
        }
    }
    /*为参数变量重命名*/
    nc=unit.varStart+func.argList.count;
    for(int i=unit.varStart;i<nc;i++){
        vm->stack[i].hashName=func.argList.vals[i].hashName;
    }
    if(func.exe!=NULL){
        func.exe(vm,&unit);
    }else{
        execute(vm,&unit);
    }
    Object*rt=POP();
    Object*obj;
    LIST_REDUCE(vm->loopList,int,vm->loopList.count-loopc)
    while(vm->stackCount>unit.varStart){
        obj=POP();
        reduceRef(vm,obj);
    }
    PUSH(rt);
}
void callClass(VM*vm,Class*class,int argc){
    Object*obj=NULL;
    obj=(Object*)memManage(obj,sizeof(Object));
    obj->class=class;
    obj->varCount=class->varList.count;
    obj->refCount=1;
    obj->objs=NULL;
    obj->isInit=true;
    if(obj->varCount>0){
        obj->objs=(Object**)memManage(obj->objs,obj->varCount*sizeof(Object*));
    }
    /*execute initFunc*/
    callInitFunc(vm,class,obj);
    if(class->initID>=0){
        obj->refCount++;
        /*塞this到参数前*/
        vm->stackCount++;
        for(int i=0;i<argc;i++){
            vm->stack[vm->stackCount-1-i]=vm->stack[vm->stackCount-2-i];
        }
        vm->stack[vm->stackCount-1-argc].obj=obj;
        Object*cf=obj->objs[class->initID];
        if(!compareClassStd(vm,cf,CLASS_FUNCTION)){
            vmError(vm,"expected a initialization method.");
        }
        callFunction(vm,cf->func,argc+1);
        cf=POP();
        reduceRef(vm,cf);
    }
    PUSH(obj);
}
void invertOrder(VM*vm,int count){
    checkStack(count);
    Stack st;
    int x=count/2;
    for(int i=0;i<x;i++){
        st=vm->stack[vm->stackCount-1-i];
        vm->stack[vm->stackCount-1-i]=vm->stack[vm->stackCount-count+i];
        vm->stack[vm->stackCount-count+i]=st;
    }
}
Object copyObject(VM*vm,Object*obj,int refCount){
    Object val=*obj;
    val.objs=NULL;
    /*refCount与前面保持一致，可能别的地方还在引用*/
    val.refCount=refCount;
    val.isInit=false;
    if(compareClassStd(vm,obj,CLASS_STRING)){
        val.str=NULL;
        val.str=(char*)memManage(val.str,strlen(obj->str)+1);
        strcpy(val.str,obj->str);
    }
    val.objs=(val.varCount>0)?(Object**)memManage(val.objs,val.varCount*sizeof(Object*)):NULL;
    for(int i=0;i<val.varCount;i++){
        val.objs[i]=NULL;
        val.objs[i]=(Object*)memManage(val.objs[i],sizeof(Object));
        *(val.objs[i])=copyObject(vm,obj->objs[i],1);
    }
    return val;
}
#define EXE_OPT(opt) \
    if(compareClassStd(vm,a,CLASS_INT)){\
        if(compareClassStd(vm,b,CLASS_INT)){\
            c=newObjectStd(vm,CLASS_INT);\
            c->num=a->num opt b->num;\
        }else if(compareClassStd(vm,b,CLASS_DOUBLE)){\
            c=newObjectStd(vm,CLASS_DOUBLE);\
            c->numd=a->num opt b->numd;\
        }else{\
            vmError(vm,"unsupported operation for int.");\
        }\
        PUSH(c);\
    }else if(compareClassStd(vm,a,CLASS_DOUBLE)){\
        c=newObjectStd(vm,CLASS_DOUBLE);\
        if(compareClassStd(vm,b,CLASS_INT)){\
            c->numd=a->numd opt b->num;\
        }else if(compareClassStd(vm,b,CLASS_DOUBLE)){\
            c->numd=a->numd opt b->numd;\
        }else{\
            vmError(vm,"unsupported operation for double.");\
        }\
        PUSH(c);\
    }else{\
        isFunc=true;\
    }
#define EXE_OPT_BIT(opt) \
    if(compareClassStd(vm,a,CLASS_INT)){\
        if(compareClassStd(vm,b,CLASS_INT)){\
            c=newObjectStd(vm,CLASS_INT);\
            c->num=a->num opt b->num;\
        }else{\
            vmError(vm,"unsupported operation for int.");\
        }\
        PUSH(c);\
    }else{\
        isFunc=true;\
    }
void exeOpt(VM*vm,int opcode){
    checkStack(2);
    Object*a,*b,*c=NULL;
    b=POP();
    a=POP();
    bool isFunc=false;
    switch(opcode){
        case OPCODE_ADD:
            EXE_OPT(+)
            break;
        case OPCODE_SUB:
            EXE_OPT(-)
            break;
        case OPCODE_MUL:
            EXE_OPT(*)
            break;
        case OPCODE_DIV:
            EXE_OPT(/)
            break;
        case OPCODE_AND:
            EXE_OPT_BIT(&)
            break;
        case OPCODE_OR:
            EXE_OPT_BIT(|)
            break;
        case OPCODE_CAND:
            EXE_OPT(&&)
            break;
        case OPCODE_COR:
            EXE_OPT(||)
            break;
        case OPCODE_LEFT:
            EXE_OPT_BIT(<<)
            break;
        case OPCODE_RIGHT:
            EXE_OPT_BIT(>>)
            break;
        case OPCODE_EQUAL:
            EXE_OPT(==)
            break;
        case OPCODE_GTHAN:
            EXE_OPT(>)
            break;
        case OPCODE_LTHAN:
            EXE_OPT(<)
            break;
        case OPCODE_NOT_EQUAL:
            EXE_OPT(!=)
            break;
        case OPCODE_GTHAN_EQUAL:
            EXE_OPT(>=)
            break;
        case OPCODE_LTHAN_EQUAL:
            EXE_OPT(<=)
            break;
        case OPCODE_REM:
            EXE_OPT_BIT(%)
            break;
        default:
            break;
    }
    if(isFunc){
        PUSH(a);
        PUSH(b);
        int ind=a->class->optID[opcode-1];
        if(ind<0){
            vmError(vm,"undefined operation.");
        }
        c=a->objs[ind];
        if(!compareClassStd(vm,c,CLASS_FUNCTION)){
            vmError(vm,"expected operation method.");
        }
        callFunction(vm,c->func,2);
    }else{
        reduceRef(vm,b);
        reduceRef(vm,a);
    }
}
/*只删除现有数据，保持指针有效*/
void delObj(VM*vm,Object*obj){
    if(obj->class->destroyID>=0){
        obj->refCount=2;
        Object*rt=obj->objs[obj->class->destroyID];
        if(!compareClassStd(vm,rt,CLASS_FUNCTION)){
            vmError(vm,"expected destroy method.");
        }
        PUSH(obj);
        callFunction(vm,rt->func,1);
        reduceRef(vm,POP());
    }
    if(compareClassStd(vm,obj,CLASS_STRING)){
        free(obj->str);
    }
    for(int i=0;i<obj->varCount;i++){
        reduceRef(vm,obj->objs[i]);
    }
    if(obj->varCount>0){
        free(obj->objs);
    }
}
void assign(VM*vm,int astype,int asc){
    checkStack(asc+1);
    Object*obj=POP();
    Object*val,*obj2;
    int hashName,refCount;
    for(int i=0;i<asc;i++){
        if(astype==-1){
            hashName=vm->stack[vm->stackCount-1].hashName;
            val=POP();
            refCount=val->refCount;
            delObj(vm,val);
            if(obj->isInit){
                *val=*obj;
                val->isInit=false;
            }else{
                *val=copyObject(vm,obj,refCount);
            }
            if(compareClassStd(vm,val,CLASS_CLASS)){
                val->classd->hashName=hashName;
            }
        }else{
            obj->refCount++;
            val=vm->stack[vm->stackCount-1].obj;
            refCount=val->refCount;
            val->refCount++;
            PUSH(obj);
            exeOpt(vm,astype);
            delObj(vm,val);
            obj2=POP();
            *val=copyObject(vm,obj2,refCount);
            reduceRef(vm,obj2);
        }
    }
    if(astype==-1 && obj->isInit){
        free(obj);
    }else{
        reduceRef(vm,obj);
    }
}
#define EXE_OPT_PREFIX(opt) \
    this=POP();\
    if(compareClassStd(vm,this,CLASS_INT)){\
        obj=newObjectStd(vm,CLASS_INT);\
        obj->num= opt this->num;\
    }else if(compareClassStd(vm,this,CLASS_DOUBLE)){\
        obj=newObjectStd(vm,CLASS_DOUBLE);\
        obj->numd= opt this->numd;\
    }else{\
        vmError(vm,"unsupported prefix operation.");\
    }\
    PUSH(obj);\
    reduceRef(vm,this);
void execute(VM*vm,Unit*unit){
    char temp[64];
    int c=0;
    Object*obj=NULL,*this=NULL;
    int asc=1;
    for(int i=0;i<unit->clist.count;i++){
        c=unit->clist.vals[i];
        if(c>=0){
            vm->part=unit->plist.vals[c];
        }
        vm->ptr=i;
        //printf("cmd:%d\n",i);
        c=unit->clist.vals[++i];
        switch(c){
            case OPCODE_NOP:
                break;
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_CAND:
            case OPCODE_COR:
            case OPCODE_LEFT:
            case OPCODE_RIGHT:
            case OPCODE_EQUAL:
            case OPCODE_GTHAN:
            case OPCODE_LTHAN:
            case OPCODE_NOT_EQUAL:
            case OPCODE_GTHAN_EQUAL:
            case OPCODE_LTHAN_EQUAL:
            case OPCODE_REM:
                exeOpt(vm,c);
                break;
            case OPCODE_INVERT:
                this=POP();
                if(compareClassStd(vm,this,CLASS_INT)){
                    obj=newObjectStd(vm,CLASS_INT);
                    obj->num=~this->num;
                }else{
                    vmError(vm,"unsupported prefix operation.");
                }
                PUSH(obj);
                reduceRef(vm,this);
                break;
            case OPCODE_NOT:
                EXE_OPT_PREFIX(!)
                break;
            case OPCODE_SUBS:
                EXE_OPT_PREFIX(-)
                break;
            case OPCODE_LOAD_CONST:
                PUSH(loadConst(vm,unit,unit->clist.vals[++i]));
                break;
            case OPCODE_LOAD_VAR:
                obj=vm->stack[unit->varStart+unit->clist.vals[++i]].obj;
                obj->refCount++;
                PUSH(obj);
                break;
            case OPCODE_LOAD_ATTR:
                this=POP();
                obj=this->objs[unit->clist.vals[++i]];
                obj->refCount++;
                reduceRef(vm,this);
                PUSH(obj);
                break;
            case OPCODE_LOAD_MEMBER:{
                this=POP();
                int ct=this->class->varList.count;
                int hashName=unit->mblist.vals[unit->clist.vals[++i]].hashName;
                bool isFound=false;
                for(int i2=0;i2<ct;i2++){
                    if(this->class->varList.vals[i2].hashName==hashName){
                        obj=this->objs[i2];
                        obj->refCount++;
                        PUSH(obj);
                        reduceRef(vm,this);
                        isFound=true;
                        break;
                    }
                }
                if(!isFound){
                    sprintf(temp,"member \"%s\" not found.",unit->mblist.vals[unit->clist.vals[i]].name);
                    vmError(vm,temp);
                }
                break;
            }
            case OPCODE_LOAD_SUBSCRIPT:
                this=vm->stack[vm->stackCount-2].obj;
                if(this->class->subID<0){
                    vmError(vm,"unsupported subscript operation.");
                }
                obj=this->objs[this->class->subID];
                if(!compareClassStd(vm,obj,CLASS_FUNCTION)){
                    vmError(vm,"expected subscript method.");
                }
                callFunction(vm,obj->func,2);
                break;
            case OPCODE_STACK_COPY:{
                Stack st=vm->stack[vm->stackCount-unit->clist.vals[++i]-1];
                st.obj->refCount++;
                vm->stack[vm->stackCount++]=st;
                break;
            }
            case OPCODE_POP_STACK:
                popStack(vm,unit->clist.vals[++i]);
                break;
            case OPCODE_JUMP:
                i++;
                i=unit->clist.vals[i]-1;
                break;
            case OPCODE_JUMP_IF_FALSE:
                obj=POP();
                if(!compareClassStd(vm,obj,CLASS_INT)){
                    vmError(vm,"expected condition value.");
                }
                i++;
                if(!obj->num){
                    i=unit->clist.vals[i]-1;
                }
                reduceRef(vm,obj);
                break;
            case OPCODE_LOAD_FIELD:{
                Stack st;
                Var var;
                VarList vlist=unit->flist.vals[unit->clist.vals[++i]].varList;
                for(int i2=0;i2<vlist.count;i2++){
                    var=vlist.vals[i2];
                    st.hashName=var.hashName;
                    if(var.isRef){
                        bool isFound=false;
                        for(int i3=unit->varStart-1;i3>=0;i3--){
                            if(vm->stack[i3].hashName==var.hashName){
                                isFound=true;
                                st.obj=vm->stack[i3].obj;
                                st.obj->refCount++;
                                break;
                            }
                        }
                        if(!isFound){
                            sprintf(temp,"unfound upvalue \"%s\".",var.name);
                            vmError(vm,temp);
                        }
                    }else{
                        obj=newObjectStd(vm,CLASS_INT);
                        obj->num=0;
                        st.obj=obj;
                    }
                    vm->stack[vm->stackCount++]=st;
                }
                break;
            }
            case OPCODE_FREE_FIELD:
                popStack(vm,unit->flist.vals[unit->clist.vals[++i]].varList.count);
                break;
            case OPCODE_SET_LOOP:
                LIST_ADD(vm->loopList,int,vm->stackCount-1)
                break;
            case OPCODE_FREE_LOOP:
                c=vm->loopList.vals[vm->loopList.count-1]+1;
                while(vm->stackCount>c){
                    obj=POP();
                    reduceRef(vm,obj);
                }
                LIST_SUB(vm->loopList,int)
                break;
            case OPCODE_CALL_FUNCTION:
                obj=POP();
                LIST_ADD(vm->plist,Part,vm->part)
                c=unit->clist.vals[++i];
                if(compareClassStd(vm,obj,CLASS_FUNCTION)){
                    callFunction(vm,obj->func,c);
                }else if(compareClassStd(vm,obj,CLASS_CLASS)){
                    callClass(vm,obj->classd,c);
                }else{
                    vmError(vm,"expected a function or class.");
                }
                LIST_SUB(vm->plist,Part)
                reduceRef(vm,obj);
                break;
            case OPCODE_RETURN:
                return;
            case OPCODE_INVERT_ORDER:
                invertOrder(vm,unit->clist.vals[++i]);
                break;
            case OPCODE_SET_ASSIGN_COUNT:
                asc=unit->clist.vals[++i];
                break;
            case OPCODE_ASSIGN:
                assign(vm,unit->clist.vals[++i],asc);
                asc=1;
                break;
            case OPCODE_MAKE_ARRAY:{
                c=unit->clist.vals[++i];
                this=newObjectStd(vm,CLASS_LIST);
                this->varCount+=c;
                this->objs=(Object**)memManage(this->objs,this->varCount);
                for(int i2=0;i2<c;i2++){
                    this->objs[this->varCount-1-i]=POP();
                }
                LIST_COUNT(this)=c;
                PUSH(this);
                break;
            }
            case OPCODE_GET_FOR_INDEX:{
                checkStack(3);
                Object*ind=vm->stack[vm->stackCount-3].obj;
                Object*array=vm->stack[vm->stackCount-2].obj;
                Object*sum=vm->stack[vm->stackCount-1].obj;
                if(!compareClassStd(vm,sum,CLASS_INT)){
                    vmError(vm,"the for index must be integer.");
                }
                if(!compareClassStd(vm,array,CLASS_LIST)){
                    vmError(vm,"expected list in for statement.");
                }
                Object*rt=newObjectStd(vm,CLASS_INT);
                if(sum->num<LIST_COUNT(array)){
                    *ind=array[array->class->varList.count+(sum->num++)];
                    ind->refCount=2;
                    rt->num=true;
                }else{
                    popStack(vm,3);
                    rt->num=false;
                }
                PUSH(rt);
                break;
            }
            case OPCODE_LOAD_STACK:
                obj=POP();
                if(!compareClassStd(vm,obj,CLASS_INT)){
                    vmError(vm,"expected integer for argument.");
                }
                this=vm->stack[unit->varStart+obj->num].obj;
                this->refCount++;
                reduceRef(vm,obj);
                PUSH(this);
                break;
            case OPCODE_LOAD_ARG_COUNT:
                obj=newObjectStd(vm,CLASS_INT);
                obj->num=unit->argc;
                PUSH(obj);
                break;
            default:
                sprintf(temp,"unknown opcode %d.",c);
                vmError(vm,temp);
                break;
        }
    }
}