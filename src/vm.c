#include"vm.h"
void initStd(VM*vm,Class meta){
    Class class;
    class.varBase=0;
    class.var.count=0;
    class.methods.count=0;
    for(int i=0;i<OPT_METHOD_COUNT;i++){
        class.optMethod[i].clist.count=0;
    }
    class.name=(char*)malloc(4);
    strcpy(class.name,"int");
    LIST_ADD(vm->classList,Class,class)
    class.name=(char*)malloc(5);
    strcpy(class.name,"class");
    LIST_ADD(vm->classList,Class,class)
    class.name=(char*)malloc(8);
    strcpy(class.name,"function");
    LIST_ADD(vm->classList,Class,class)
    LIST_ADD(vm->classList,Class,meta)
    Parser parser;
    initParser(&parser,true);
    import(&parser,"lib/float.pdl");
    extend(&parser.classList.vals[0],meta);
    import(&parser,"lib/string.pdl");
    extend(&parser.classList.vals[1],meta);
    LIST_CONNECT(vm->classList,parser.classList,Class,0)
    freeParser(&parser);
}
void initVM(VM*vm,Parser parser){
    vm->partList=parser.partList;
    vm->clist=parser.clist;
    vm->symList=parser.symList;
    vm->funcList=parser.funcList;
    vm->moduleList=parser.moduleList;
    vm->curModule=vm->moduleList.vals[0];
    LIST_INIT(vm->classList,Class)
    initStd(vm,parser.meta);
    LIST_CONNECT(vm->classList,parser.classList,Class,0)
    LIST_INIT(vm->stack,Value)
    LIST_INIT(vm->refList,Ref)
    LIST_INIT(vm->varList,Var)
    LIST_INIT(vm->enableFunc,int)
    LIST_INIT(vm->fields,Field)
    LIST_INIT(vm->retFieldList,int)
    LIST_INIT(vm->retLoopList,int)
    LIST_INIT(vm->loopList,int)
    LIST_INIT(vm->mlist,Module)
    LIST_INIT(vm->funcPartList,int)
    vm->ptr=0;
}
void reportVMError(VM*vm,char*text,int curPart){
    Msg msg;
    Part part;
    char*str;
    for(int i=0;i<vm->funcPartList.count;i++){
        part=vm->partList.vals[vm->funcPartList.vals[i]];
        str=cutText(part.code,part.start,part.end);
        printf("from %s:%d:%d:\n    %s\n",part.fileName,part.line,part.column,str);
        free(str);
    }
    msg.type=MSG_ERROR;
    part=vm->partList.vals[curPart];
    msg.fileName=part.fileName;
    msg.code=part.code;
    msg.start=part.start;
    msg.end=part.end;
    msg.line=part.line;
    msg.column=part.column;
    strcpy(msg.text,text);
    reportMsg(msg);
}
int getVMClass(VM*vm,int class){
    if(class<0){
        return -class-1;
    }
    return vm->curModule.classBase+class;
}
Value makeValue(VM*vm,int class){
    Value val;
    Class classd;
    Ref ref;
    val.class=class;
    classd=vm->classList.vals[val.class];
    ref.varCount=classd.var.count;
    ref.varBase=classd.varBase;
    if(ref.varCount>0){
        ref.var=(Value*)malloc(ref.varCount*sizeof(Value));
        ref.refCount=1;
        LIST_ADD(vm->refList,Ref,ref)
        val.refID=vm->refList.count-1;
    }else{
        val.refID=-1;
    }
    return val;
}
static void checkStack(VM*vm,int count,int curPart){
    char temp[50];
    if(vm->stack.count<count){
        sprintf(temp,"in %d:it need %d slots but only %d slots in stack.",vm->ptr,count,vm->stack.count);
        reportVMError(vm,temp,curPart);
    }
}
static void addRef(VM*vm,Value value){
    if(value.refID>=0){
        vm->refList.vals[value.refID].refCount++;
    }
}
static void reduceRef(VM*vm,Value value){
    if(value.refID>0){
        vm->refList.vals[value.refID].refCount--;
        if(vm->refList.vals[value.refID].refCount<=0){
            free(vm->refList.vals[value.refID].var);
        }
    }
}
inline static void printStack(VM*vm){
    Value val;
    char temp[20];
    for(int i=0;i<vm->stack.count;i++){
        val=vm->stack.vals[i];
        if(val.class==getVMClass(vm,CLASS_INT) || val.class==getVMClass(vm,CLASS_CLASS) || val.class==getVMClass(vm,CLASS_FUNCTION)){
            sprintf(temp,"%d",val.num);
        }else if(val.class==getVMClass(vm,CLASS_FLOAT)){
            sprintf(temp,"%f",val.numf);
        }else if(val.class==getVMClass(vm,CLASS_STRING)){
            sprintf(temp,"%s",val.str);
        }else{
            strcpy(temp,"object");
        }
        printf("stack%d:class:%s,value:%s,refID:%d\n",i,vm->classList.vals[val.class].name,temp,val.refID);
    }
}
inline static void printVar(VM*vm){
    Var var;
    char temp[20];
    for(int i=0;i<vm->varList.count;i++){
        var=vm->varList.vals[i];
        if(var.val.class==getVMClass(vm,CLASS_INT) || var.val.class==getVMClass(vm,CLASS_CLASS) || var.val.class==getVMClass(vm,CLASS_FUNCTION)){
            sprintf(temp,"%d",var.val.num);
        }else if(var.val.class==getVMClass(vm,CLASS_FLOAT)){
            sprintf(temp,"%f",var.val.numf);
        }else if(var.val.class==getVMClass(vm,CLASS_STRING)){
            sprintf(temp,"%s",var.val.str);
        }else{
            strcpy(temp,"object");
        }
        printf("variable%d(%s):class:%s,value:%s,refID:%d\n",i,var.name,vm->classList.vals[var.val.class].name,temp,var.val.refID);
    }
}
inline static void loadConst(VM*vm,int arg,int curPart){
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBase];
    Value value;
    switch (symbol.type)
    {
    case SYM_INT:
        value.class=CLASS_INT;
        break;
    case SYM_FLOAT:
        value.class=CLASS_FLOAT;
        break;
    case SYM_STRING:
        value.class=CLASS_STRING;
        break;
    default:
        reportVMError(vm,"LOAD_CONST:unknown symbol type.",curPart);
        break;
    }
    value=makeValue(vm,getVMClass(vm,value.class));
    switch (symbol.type)
    {
    case SYM_INT:
        value.num=symbol.num;
        break;
    case SYM_FLOAT:
        value.numf=symbol.numf;
        break;
    case SYM_STRING:
        value.str=symbol.str;
        break;
    default:
        reportVMError(vm,"LOAD_CONST:unknown symbol type.",curPart);
        break;
    }
    LIST_ADD(vm->stack,Value,value)
}
inline static void loadVal(VM*vm,int arg,int curPart){
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBase];
    char temp[50];
    if(symbol.type!=SYM_STRING){
        reportVMError(vm,"LOAD_VAL:symbol type must be string.",curPart);
    }
    bool isFound=false;
    for(int i=vm->varList.count-1;i>=0;i--){
        if(strcmp(symbol.str,vm->varList.vals[i].name)==0){
            addRef(vm,vm->varList.vals[i].val);
            LIST_ADD(vm->stack,Value,vm->varList.vals[i].val)
            isFound=true;
            break;
        }
    }
    if(!isFound){
        sprintf(temp,"variable \"%s\" not found.",symbol.str);
        reportVMError(vm,temp,curPart);
    }
}
inline static void loadAttr(VM*vm,int arg,int curPart){
    checkStack(vm,1,curPart);
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBase];
    char temp[50];
    if(symbol.type!=SYM_STRING){
        reportVMError(vm,"LOAD_ATTR:symbol type must be string.",curPart);
    }
    Value a=vm->stack.vals[vm->stack.count-1];
    if(a.refID<0){
        sprintf(temp,"member \"%s\" not found and no memory is alloced.",symbol.str);
        reportVMError(vm,temp,curPart);
    }
    Ref ref=vm->refList.vals[a.refID];
    bool isFound=false;
    for(int i=0;i<vm->classList.vals[a.class].var.count;i++){
        if(strcmp(symbol.str,vm->classList.vals[a.class].var.vals[i])==0){
            isFound=true;
            vm->stack.vals[vm->stack.count-1]=ref.var[i];
            addRef(vm,ref.var[i]);
            reduceRef(vm,a);
            break;
        }
    }
    if(!isFound){
        sprintf(temp,"member \"%s\" not found.",symbol.str);
        reportVMError(vm,temp,curPart);
    }
}
inline static void loadIndex(VM*vm,int curPart){
    Value a,b;
    checkStack(vm,2,curPart);
    a=vm->stack.vals[vm->stack.count-2];
    b=vm->stack.vals[vm->stack.count-1];
    if(a.refID<0){
        reportVMError(vm,"the variable is not an array.",curPart);
    }
    if(b.class!=getVMClass(vm,CLASS_INT)){
        reportVMError(vm,"it must be integer when calling array.",curPart);
    }
    Ref ref=vm->refList.vals[a.refID];
    int ind=b.num+ref.varBase;
    if((ind<0 || ind>=ref.varCount) && a.class!=getVMClass(vm,CLASS_STRING)){
        reportVMError(vm,"illegal array index.",curPart);
    }
    LIST_SUB(vm->stack,Value)
    if(a.class==getVMClass(vm,CLASS_STRING)){
        if(b.num>=strlen(a.str)){
            reportVMError(vm,"illegal string index.",curPart);
        }
        Value val;
        val.class=getVMClass(vm,CLASS_INT);
        val.num=a.str[b.num];
        vm->stack.vals[vm->stack.count-1]=val;
    }else{
        vm->stack.vals[vm->stack.count-1]=ref.var[ind];
    }
    vm->stack.vals[vm->stack.count-1]=ref.var[ind];
    addRef(vm,ref.var[ind]);
    reduceRef(vm,a);
    reduceRef(vm,b);
}
inline static void storeVal(VM*vm,int arg,int curPart){
    checkStack(vm,1,curPart);
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBase];
    if(symbol.type!=SYM_STRING){
        reportVMError(vm,"STORE_VAL:symbol type must be string.",curPart);
    }
    bool isFound=false;
    for(int i=vm->varList.count-1;i>=0;i--){
        if(strcmp(vm->varList.vals[i].name,symbol.str)==0){
            isFound=true;
            vm->varList.vals[i].val=vm->stack.vals[vm->stack.count-1];
            LIST_SUB(vm->stack,Value)
            break;
        }
    }
    if(!isFound){
        Var var;
        var.name=symbol.str;
        var.val=vm->stack.vals[vm->stack.count-1];
        LIST_ADD(vm->varList,Var,var)
        LIST_SUB(vm->stack,Value)
    }
}
inline static void storeAttr(VM*vm,int arg,int curPart){
    checkStack(vm,2,curPart);
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBase];
    char temp[50];
    if(symbol.type!=SYM_STRING){
        reportVMError(vm,"STORE_ATTR:symbol type must be string.",curPart);
    }
    Value a=vm->stack.vals[vm->stack.count-1];
    if(a.refID<0){
        sprintf(temp,"member \"%s\" not found and no memory is alloced.",symbol.str);
        reportVMError(vm,temp,curPart);
    }
    Ref ref=vm->refList.vals[a.refID];
    bool isFound=false;
    for(int i=0;i<vm->classList.vals[a.class].var.count;i++){
        if(strcmp(symbol.str,vm->classList.vals[a.class].var.vals[i])==0){
            isFound=true;
            ref.var[i]=vm->stack.vals[vm->stack.count-2];
            LIST_SUB(vm->stack,Value)
            LIST_SUB(vm->stack,Value)
            reduceRef(vm,a);
            break;
        }
    }
    if(!isFound){
        sprintf(temp,"member \"%s\" not found.",symbol.str);
        reportVMError(vm,temp,curPart);
    }
}
inline static void storeIndex(VM*vm,int curPart){
    Value a,b,c;
    checkStack(vm,3,curPart);
    a=vm->stack.vals[vm->stack.count-2];
    b=vm->stack.vals[vm->stack.count-1];
    c=vm->stack.vals[vm->stack.count-3];
    if(a.refID<=0){
        reportVMError(vm,"the variable is not an array.",curPart);
    }
    if(b.class!=getVMClass(vm,CLASS_INT)){
        reportVMError(vm,"it must be integer when calling array.",curPart);
    }
    Ref ref=vm->refList.vals[a.refID];
    int ind=b.num+ref.varBase;
    if((ind<0 || ind>=ref.varCount) && a.class!=getVMClass(vm,CLASS_STRING)){
        reportVMError(vm,"illegal array index.",curPart);
    }
    if(a.class==getVMClass(vm,CLASS_STRING)){
        if(b.num>=strlen(a.str)){
            reportVMError(vm,"illegal string index.",curPart);
        }
        a.str[b.num]=c.num;
    }else{
        ref.var[ind]=c;
    }
    reduceRef(vm,a);
    reduceRef(vm,b);
    LIST_SUB(vm->stack,Value)
    LIST_SUB(vm->stack,Value)
    LIST_SUB(vm->stack,Value)
}
inline static void pushVal(VM*vm,int arg,int curPart){
    checkStack(vm,1,curPart);
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBase];
    if(symbol.type!=SYM_STRING){
        reportVMError(vm,"PUSH_VAL:symbol type must be string.",curPart);
    }
    Var var;
    var.name=symbol.str;
    var.val=vm->stack.vals[vm->stack.count-1];
    LIST_ADD(vm->varList,Var,var)
    LIST_SUB(vm->stack,Value)
}
inline static void stackCopy(VM*vm,int arg,int curPart){
    checkStack(vm,arg+1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1-arg];
    LIST_ADD(vm->stack,Value,val)
    addRef(vm,val);
}
inline static void popVar(VM*vm,int arg,int curPart){
    checkStack(vm,arg,curPart);
    for(int i=0;i<arg;i++){
        reduceRef(vm,vm->varList.vals[vm->varList.count-i-1].val);
        LIST_SUB(vm->varList,Var)
    }
}
inline static void popStack(VM*vm,int arg,int curPart){
    checkStack(vm,arg,curPart);
    for(int i=0;i<arg;i++){
        reduceRef(vm,vm->stack.vals[vm->stack.count-i-1]);
        LIST_SUB(vm->stack,Value)
    }
}
inline static void setField(VM*vm){
    Field field;
    field.varIndex=vm->varList.count-1;
    field.funcIndex=vm->enableFunc.count-1;
    LIST_ADD(vm->fields,Field,field)
}
inline static void freeField(VM*vm){
    Field field=vm->fields.vals[vm->fields.count-1];
    LIST_REDUCE(vm->varList,Var,vm->varList.count-field.varIndex-1)
    LIST_REDUCE(vm->enableFunc,int,vm->enableFunc.count-field.funcIndex-1)
    LIST_SUB(vm->fields,Field)
}
inline static void setLoop(VM*vm){
    LIST_ADD(vm->loopList,int,vm->fields.count-1)
}
inline static void freeLoop(VM*vm){
    int top=vm->loopList.vals[vm->loopList.count-1];
    while(vm->fields.count>top+1){
        freeField(vm);
    }
    LIST_SUB(vm->loopList,int)
}
inline static void callFunction(VM*vm,int arg,int curPart){
    checkStack(vm,arg+1,curPart);
    char temp[50];
    Value a=vm->stack.vals[vm->stack.count-arg-1];
    if(a.class!=getVMClass(vm,CLASS_STRING)){
        reportVMError(vm,"CALL_FUNCTION:expected function name.",curPart);
    }
    Func func;
    func.args.vals=NULL;
    func.args.count=0;
    func.moduleID=0;
    bool isFound=false;
    for(int i=0;i<vm->enableFunc.count;i++){
        func=vm->funcList.vals[vm->enableFunc.vals[i]];
        if(strcmp(a.str,func.name)==0){
            isFound=true;
            break;
        }
    }
    if(!isFound){
        Var var;
        for(int i=0;i<vm->varList.count;i++){
            var=vm->varList.vals[i];
            if(var.val.class==getVMClass(vm,CLASS_FUNCTION)){
                if(strcmp(var.name,a.str)==0){
                    func=vm->funcList.vals[var.val.num];
                    isFound=true;
                    break;
                }
            }
        }
        if(!isFound){
            sprintf(temp,"function \"%s\" not found.",a.str);
            reportVMError(vm,temp,curPart);
        }
    }
    LIST_ADD(vm->mlist,Module,vm->curModule)
    vm->curModule=vm->moduleList.vals[func.moduleID];
    Var var;
    for(int i=0;i<arg && i<func.args.count;i++){
        var.val=vm->stack.vals[vm->stack.count-arg+i];
        var.name=func.args.vals[i];
        LIST_ADD(vm->varList,Var,var)
    }
    LIST_REDUCE(vm->stack,Value,arg+1)
    LIST_ADD(vm->retFieldList,int,vm->fields.count-1)
    LIST_ADD(vm->retLoopList,int,vm->loopList.count-1)
    LIST_ADD(vm->funcPartList,int,curPart)
    execute(vm,func.clist);
    LIST_SUB(vm->funcPartList,int)
    LIST_REDUCE(vm->varList,Var,arg)
    vm->curModule=vm->mlist.vals[vm->mlist.count-1];
    LIST_SUB(vm->mlist,Module)
    int top=vm->retLoopList.vals[vm->retLoopList.count-1];
    while(vm->loopList.count>top+1){
        freeLoop(vm);
    }
    top=vm->retFieldList.vals[vm->retFieldList.count-1];
    while(vm->fields.count>top+1){
        freeField(vm);
    }
    LIST_SUB(vm->retLoopList,int)
    LIST_SUB(vm->retFieldList,int)
}
inline static void callMethod(VM*vm,int arg,int curPart){
    checkStack(vm,arg+2,curPart);
    char temp[50];
    Value a=vm->stack.vals[vm->stack.count-arg-2];
    Value b=vm->stack.vals[vm->stack.count-arg-1];
    if(b.class!=getVMClass(vm,CLASS_STRING)){
        reportVMError(vm,"CALL_FUNCTION:expected function name.",curPart);
    }
    Func func;
    func.args.vals=NULL;
    func.args.count=0;
    func.moduleID=0;
    bool isFound=false;
    for(int i=0;i<vm->classList.vals[a.class].methods.count;i++){
        func=vm->classList.vals[a.class].methods.vals[i];
        if(strcmp(b.str,func.name)==0){
            isFound=true;
            break;
        }
    }
    if(!isFound){
        sprintf(temp,"method \"%s\" not found.",b.str);
        reportVMError(vm,temp,curPart);
    }
    LIST_ADD(vm->mlist,Module,vm->curModule)
    vm->curModule=vm->moduleList.vals[func.moduleID];
    Var var;
    var.val=a;
    var.name=(char*)malloc(5);
    strcpy(var.name,"this");
    LIST_ADD(vm->varList,Var,var)
    for(int i=0;i<arg && i<func.args.count;i++){
        var.val=vm->stack.vals[vm->stack.count-arg+i];
        var.name=func.args.vals[i];
        LIST_ADD(vm->varList,Var,var)
    }
    LIST_REDUCE(vm->stack,Value,arg+2)
    LIST_ADD(vm->retFieldList,int,vm->fields.count-1)
    LIST_ADD(vm->retLoopList,int,vm->loopList.count-1)
    LIST_ADD(vm->funcPartList,int,curPart)
    execute(vm,func.clist);
    LIST_SUB(vm->funcPartList,int)
    LIST_REDUCE(vm->varList,Var,arg+1)
    vm->curModule=vm->mlist.vals[vm->mlist.count-1];
    LIST_SUB(vm->mlist,Module)
    int top=vm->retLoopList.vals[vm->retLoopList.count-1];
    while(vm->loopList.count>top+1){
        freeLoop(vm);
    }
    top=vm->retFieldList.vals[vm->retFieldList.count-1];
    while(vm->fields.count>top+1){
        freeField(vm);
    }
    LIST_SUB(vm->retLoopList,int)
    LIST_SUB(vm->retFieldList,int)
}
inline static void enableFunction(VM*vm,int arg){
    LIST_ADD(vm->enableFunc,int,arg+vm->curModule.funcBase)
    Var var;
    var.name=vm->funcList.vals[arg+vm->curModule.funcBase].name;
    var.val=makeValue(vm,getVMClass(vm,CLASS_FUNCTION));
    var.val.num=arg+vm->curModule.funcBase;
    LIST_ADD(vm->varList,Var,var)
}
inline static void makeObject(VM*vm,int arg){
    Value val=makeValue(vm,getVMClass(vm,arg));
    LIST_ADD(vm->stack,Value,val)
}
inline static void extendClass(VM*vm,int arg,int curPart){
    checkStack(vm,1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1];
    if(val.class!=getVMClass(vm,CLASS_INT)){
        reportVMError(vm,"the variable must be integer as an index when extending class.",curPart);
    }
    extend(&vm->classList.vals[arg+vm->curModule.classBase],vm->classList.vals[val.num]);
}
inline static void enableClass(VM*vm,int arg){
    Var var;
    int class=arg+vm->curModule.classBase;
    var.name=vm->classList.vals[class].name;
    var.val=makeValue(vm,getVMClass(vm,CLASS_CLASS));
    var.val.num=class;
    LIST_ADD(vm->varList,Var,var)
}
inline static void setModule(VM*vm,int arg,int curPart){
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBase];
    if(symbol.type!=SYM_STRING){
        reportVMError(vm,"SET_MODULE:the symbol type must be string.",curPart);
    }
    bool isFound=false;
    for(int i=0;i<vm->moduleList.count;i++){
        if(strcmp(symbol.str,vm->moduleList.vals[i].name)==0){
            LIST_ADD(vm->mlist,Module,vm->curModule)
            vm->curModule=vm->moduleList.vals[i];
            isFound=true;
            break;
        }
    }
    if(!isFound){
        reportVMError(vm,"SET_MODULE:module not found.",curPart);
    }
}
inline static void returnModule(VM*vm){
    vm->curModule=vm->mlist.vals[vm->mlist.count-1];
    LIST_SUB(vm->mlist,Module)
}
static void exeOpt(VM*vm,int ind,int curPart){
    Value a,b;
    a=vm->stack.vals[vm->stack.count-2];
    b=vm->stack.vals[vm->stack.count-1];
    Var var;
    var.name=(char*)malloc(5);
    strcpy(var.name,"this");
    var.val=a;
    LIST_ADD(vm->varList,Var,var)
    var.name=vm->classList.vals[a.class].optMethod[ind].args.vals[0];
    var.val=b;
    LIST_ADD(vm->varList,Var,var)
    LIST_ADD(vm->retFieldList,int,vm->fields.count-1)
    LIST_ADD(vm->retLoopList,int,vm->loopList.count-1)
    LIST_REDUCE(vm->stack,Value,2)
    LIST_ADD(vm->funcPartList,int,curPart)
    execute(vm,vm->classList.vals[a.class].optMethod[ind].clist);
    LIST_SUB(vm->funcPartList,int)
    LIST_SUB(vm->mlist,Module)
    int top=vm->retLoopList.vals[vm->retLoopList.count-1];
    while(vm->loopList.count>top+1){
        freeLoop(vm);
    }
    top=vm->retFieldList.vals[vm->retFieldList.count-1];
    while(vm->fields.count>top+1){
        freeField(vm);
    }
    LIST_REDUCE(vm->varList,Var,2)
    vm->curModule=vm->mlist.vals[vm->mlist.count-1];
    LIST_SUB(vm->retLoopList,int)
    LIST_SUB(vm->retFieldList,int)
}
#define CINT 0
#define CFLOAT 4
#define CCLASS 2
#define EXE_OPT(opt,ind) \
    checkStack(vm,2,curPart);\
    a=vm->stack.vals[vm->stack.count-2];\
    b=vm->stack.vals[vm->stack.count-1];\
    switch(a.class){\
        case CINT:\
            switch(b.class){\
                case CINT:\
                    a.num=a.num opt b.num;\
                    break;\
                case CFLOAT:\
                    a.class=CFLOAT;\
                    a.numf=a.num opt b.numf;\
                    break;\
                default:\
                    reportVMError(vm,"unsupport int operation.",curPart);\
            }\
            LIST_SUB(vm->stack,Value)\
            vm->stack.vals[vm->stack.count-1]=a;\
            break;\
        case CFLOAT:\
            switch(b.class){\
                case CINT:\
                    a.numf=a.numf opt b.num;\
                    break;\
                case CFLOAT:\
                    a.numf=a.numf opt b.numf;\
                    break;\
                default:\
                    reportVMError(vm,"unsupport float operation.",curPart);\
            }\
            LIST_SUB(vm->stack,Value)\
            vm->stack.vals[vm->stack.count-1]=a;\
            break;\
        case CCLASS:\
            if(b.class!=CCLASS){\
                reportVMError(vm,"expected a class to equal.",curPart);\
            }\
            a.num=a.num==b.num;\
            break;\
        default:\
            if(vm->classList.vals[a.class].optMethod[ind].clist.count<=0){\
                sprintf(temp,"unsupport %s operation.",vm->classList.vals[a.class].name);\
                reportVMError(vm,temp,curPart);\
            }\
            exeOpt(vm,ind,curPart);\
    }

#define EXE_OPT_BIT(opt,ind) \
    checkStack(vm,2,curPart);\
    a=vm->stack.vals[vm->stack.count-2];\
    b=vm->stack.vals[vm->stack.count-1];\
    if(a.class==getVMClass(vm,CLASS_INT)){\
        if(b.class!=getVMClass(vm,CLASS_INT)){\
            reportVMError(vm,"expected an integer to operate.",curPart);\
        }\
        a.num= a.num opt b.num;\
        LIST_SUB(vm->stack,Value)\
        vm->stack.vals[vm->stack.count-1]=a;\
    }else{\
        if(vm->classList.vals[a.class].optMethod[ind].clist.count<=0){\
            sprintf(temp,"unsupport %s operation.",vm->classList.vals[a.class].name);\
            reportVMError(vm,temp,curPart);\
        }\
        exeOpt(vm,ind,curPart);\
    }

void execute(VM*vm,intList clist){
    int opcode,curPart;
    char temp[50];
    Value a,b;
    for(int i=0;i<clist.count;i++){
        curPart=clist.vals[i++];
        opcode=clist.vals[i];
        vm->ptr=i;
        switch(opcode){
            case OPCODE_NOP:
                break;
            case OPCODE_ADD:
                EXE_OPT(+,OPCODE_ADD)
                break;
            case OPCODE_SUB:
                EXE_OPT(-,OPCODE_SUB)
                break;
            case OPCODE_MUL:
                EXE_OPT(*,OPCODE_MUL)
                break;
            case OPCODE_DIV:
                EXE_OPT(/,OPCODE_DIV)
                break;
            case OPCODE_CAND:
                EXE_OPT(&&,OPCODE_CAND)
                break;
            case OPCODE_COR:
                EXE_OPT(||,OPCODE_COR)
                break;
            case OPCODE_AND:
                EXE_OPT_BIT(&,OPCODE_AND)
                break;
            case OPCODE_OR:
                EXE_OPT_BIT(|,OPCODE_OR)
                break;
            case OPCODE_LEFT:
                EXE_OPT_BIT(<<,OPCODE_LEFT)
                break;
            case OPCODE_RIGHT:
                EXE_OPT_BIT(>>,OPCODE_RIGHT)
                break;
            case OPCODE_EQUAL:
                EXE_OPT(==,OPCODE_EQUAL)
                break;
            case OPCODE_GTHAN:
                EXE_OPT(>,OPCODE_GTHAN)
                break;
            case OPCODE_LTHAN:
                EXE_OPT(<,OPCODE_LTHAN)
                break;
            case OPCODE_NOT_EQUAL:
                EXE_OPT(!=,OPCODE_NOT_EQUAL)
                break;
            case OPCODE_GTHAN_EQUAL:
                EXE_OPT(>=,OPCODE_GTHAN_EQUAL)
                break;
            case OPCODE_LTHAN_EQUAL:
                EXE_OPT(<=,OPCODE_LTHAN_EQUAL)
                break;
            case OPCODE_REM:
                EXE_OPT_BIT(%,OPCODE_REM)
                break;
            case OPCODE_SUBS:
                a=vm->stack.vals[vm->stack.count-1];
                if(a.class==getVMClass(vm,CLASS_INT)){
                    a.num=-a.num;
                }else if(a.class==CLASS_FLOAT){
                    a.numf=-a.numf;
                }else{
                    reportVMError(vm,"unsupported class for prefix operation \"-\".",curPart);
                }
            case OPCODE_INVERT:
                a=vm->stack.vals[vm->stack.count-1];
                if(a.class==getVMClass(vm,CLASS_INT)){
                    a.num=~a.num;
                }else{
                    reportVMError(vm,"unsupported class for prefix operation \"~\".",curPart);
                }
            case OPCODE_NOT:
                a=vm->stack.vals[vm->stack.count-1];
                if(a.class==getVMClass(vm,CLASS_INT)){
                    a.num=!a.num;
                }else if(a.class==CLASS_FLOAT){
                    a.numf=!a.numf;
                }else{
                    reportVMError(vm,"unsupported class for prefix operation \"!\".",curPart);
                }
            case OPCODE_LOAD_CONST:
                loadConst(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_LOAD_VAL:
                loadVal(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_LOAD_ATTR:
                loadAttr(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_LOAD_INDEX:
                loadIndex(vm,curPart);
                break;
            case OPCODE_STORE_VAL:
                storeVal(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_STORE_ATTR:
                storeAttr(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_STORE_INDEX:
                storeIndex(vm,curPart);
                break;
            case OPCODE_PUSH_VAL:
                pushVal(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_STACK_COPY:
                stackCopy(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_POP_VAR:
                popVar(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_POP_STACK:
                popStack(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_JUMP:
                i++;
                i=clist.vals[i]+vm->curModule.cmdBase-1;
                break;
            case OPCODE_JUMP_IF_FALSE:
                checkStack(vm,1,curPart);
                a=vm->stack.vals[vm->stack.count-1];
                if(a.class!=getVMClass(vm,CLASS_INT)){
                    reportVMError(vm,"expected an integer in condition statement.",curPart);
                }
                i++;
                if(!a.num){
                    i=clist.vals[i]+vm->curModule.cmdBase-1;
                }
                LIST_SUB(vm->stack,Value)
            case OPCODE_SET_FIELD:
                setField(vm);
                break;
            case OPCODE_FREE_FIELD:
                freeField(vm);
                break;
            case OPCODE_SET_LOOP:
                setLoop(vm);
                break;
            case OPCODE_FREE_LOOP:
                freeLoop(vm);
                break;
            case OPCODE_CALL_FUNCTION:
                callFunction(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_CALL_METHOD:
                callMethod(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_RETURN:
                return;
            case OPCODE_ENABLE_FUNCTION:
                enableFunction(vm,clist.vals[++i]);
                break;
            case OPCODE_MAKE_OBJECT:
                makeObject(vm,clist.vals[++i]);
                break;
            case OPCODE_EXTEND_CLASS:
                extendClass(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_ENABLE_CLASS:
                enableClass(vm,clist.vals[++i]);
                break;
            case OPCODE_SET_MODULE:
                setModule(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_RETURN_MODULE:
                returnModule(vm);
                break;
            case OPCODE_PRINT_STACK:
                printStack(vm);
                break;
            case OPCODE_PRINT_VAR:
                printVar(vm);
                break;
            default:
                sprintf(temp,"unknown operation code (%d).",opcode);
                reportVMError(vm,temp,curPart);
        }
    }
}