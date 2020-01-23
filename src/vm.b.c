#include"vm.h"
/*所有变量在创建时必须初始化，否则其默认的refID会造成内存紊乱*/
/*所以类必须有成员（包括继承的）才能执行destroyXXX()*/
extern OpcodeMsg opcodeList[];
void initVM(VM*vm,Parser parser){
    vm->partList=parser.partList;
    vm->clist=parser.clist;
    vm->symList=parser.symList;
    vm->funcList=parser.funcList;
    vm->moduleList=parser.moduleList;
    vm->curModule=vm->moduleList.vals[parser.curModule];
    vm->classList=parser.classList;
    vm->path=parser.path;
    LIST_INIT(vm->stack,Value)
    LIST_INIT(vm->refList,Ref)
    LIST_INIT(vm->varList,Var)
    LIST_INIT(vm->enableFunc,int)
    LIST_INIT(vm->fields,Field)
    LIST_INIT(vm->loopList,int)
    LIST_INIT(vm->mlist,Module)
    LIST_INIT(vm->funcPartList,int)
    LIST_INIT(vm->dllptrList,Dllptr)
    vm->ptr=0;
    vm->curPart=0;
    vm->curVar=-1;
    LIST_INIT(vm->vlist,int)
    Var var;
    var.val=makeValue(vm,CLASS_CLASS);
    var.name=(char*)malloc(4);
    strcpy(var.name,"int");
    var.val.num=CLASS_INT;
    LIST_ADD(vm->varList,Var,var)
    var.name=(char*)malloc(6);
    strcpy(var.name,"class");
    var.val.num=CLASS_CLASS;
    LIST_ADD(vm->varList,Var,var)
    var.name=(char*)malloc(9);
    strcpy(var.name,"function");
    var.val.num=CLASS_FUNCTION;
    LIST_ADD(vm->varList,Var,var)
}
void reportVMError(VM*vm,char*text,int curPart){
    Msg msg;
    Part part;
    char*str;
    for(int i=0;i<vm->funcPartList.count;i++){
        part=vm->partList.vals[vm->funcPartList.vals[i]];
        str=cutText(part.code,part.start,part.end);
        if(str==NULL){
            printf("from %s:%d:%d:\n",part.fileName,part.line,part.column);
        }else{
            printf("from %s:%d:%d:\n    %s\n",part.fileName,part.line,part.column,str);
            free(str);
        }
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
    exitVM(vm);
    reportMsg(msg);
}
int getClassSize(VM*vm,int class){
    Class classd=vm->classList.vals[class];
    int result=classd.var.count;
    for(int i=0;i<classd.parentList.count;i++){
        result+=getClassSize(vm,classd.parentList.vals[i]);
    }
    return result;
}
bool searchAttr(VM*vm,int*result,int class,char*name){
    Class classd=vm->classList.vals[class];
    for(int i=0;i<classd.var.count;i++){
        if(strcmp(classd.var.vals[i],name)==0){
            (*result)+=i;
            return true;
        }
    }
    (*result)+=classd.var.count;
    for(int i=0;i<classd.parentList.count;i++){
        if(searchAttr(vm,result,classd.parentList.vals[i],name)){
            return true;
        }
    }
    return false;
}
int makeRef(VM*vm,Ref ref){
    ref.isUsed=true;
    for(int i=0;i<vm->refList.count;i++){
        if(!vm->refList.vals[i].isUsed){
            vm->refList.vals[i]=ref;
            return i;
        }
    }
    LIST_ADD(vm->refList,Ref,ref)
    return vm->refList.count-1;
}
void addFuncArgs(VM*vm,int count){
    Var var;
    Ref ref;
    var.name=(char*)malloc(7);
    strcpy(var.name,"argVar");
    var.val=makeValue(vm,CLASS_INT);
    var.val.num=count;
    ref.refCount=1;
    ref.varBasis=0;
    ref.varCount=count;
    ref.str=NULL;
    ref.var=(Value*)malloc(count*sizeof(Value));
    for(int i=0;i<count;i++){
        ref.var[i]=vm->stack.vals[vm->stack.count-count+i];
        if(ref.var[i].refID>=0){
            vm->refList.vals[ref.var[i].refID].refCount++;
        }
    }
    var.val.refID=makeRef(vm,ref);
    LIST_ADD(vm->varList,Var,var)
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
    Class class;
    if(value.refID>=0){
        class=vm->classList.vals[value.class];
        vm->refList.vals[value.refID].refCount--;
        if(vm->refList.vals[value.refID].refCount<=0){
            if(class.destroyID>=0){
                vm->refList.vals[value.refID].refCount=2;
                LIST_ADD(vm->stack,Value,value)
                exeFunc(vm,class.methods.vals[class.destroyID],0,true,true,vm->curPart);
                LIST_SUB(vm->stack,Value)
            }
            for(int i=0;i<vm->refList.vals[value.refID].varCount;i++){
                reduceRef(vm,vm->refList.vals[value.refID].var[i]);
            }
            free(vm->refList.vals[value.refID].var);
            vm->refList.vals[value.refID].var=NULL;
            vm->refList.vals[value.refID].isUsed=false;
            if(vm->refList.vals[value.refID].str!=NULL){
                free(vm->refList.vals[value.refID].str);
                vm->refList.vals[value.refID].str=NULL;
            }
        }
    }
}
void exitVM(VM*vm){
    while(vm->stack.count>0){
        reduceRef(vm,vm->stack.vals[vm->stack.count-1]);
        LIST_SUB(vm->stack,Value)
    }
    while(vm->varList.count>0){
        reduceRef(vm,vm->varList.vals[vm->varList.count-1].val);
        LIST_SUB(vm->varList,Var)
    }
    for(int i=0;i<vm->dllptrList.count;i++){
        if(vm->dllptrList.vals[i]!=NULL){
            dlclose(vm->dllptrList.vals[i]);
            vm->dllptrList.vals[i]=NULL;
        }
    }
}
static void exeInit(VM*vm,int class,Value val){
    Class classd=vm->classList.vals[class];
    for(int i=0;i<classd.parentList.count;i++){
        exeInit(vm,classd.parentList.vals[i],val);
    }
    if(classd.initValID>=0){
        addRef(vm,val);
        LIST_ADD(vm->stack,Value,val)
        exeFunc(vm,classd.methods.vals[classd.initValID],0,true,true,vm->curPart);
        reduceRef(vm,vm->stack.vals[vm->stack.count-1]);
        LIST_SUB(vm->stack,Value);
    }
}
Value makeValue(VM*vm,int class){
    Value val;
    Class classd;
    Ref ref;
    val.class=class;
    classd=vm->classList.vals[val.class];
    ref.varCount=getClassSize(vm,val.class);
    ref.varBasis=classd.varBasis;
    ref.str=NULL;
    if(ref.varCount>0 || class==CLASS_STRING){
        ref.var=(Value*)malloc(ref.varCount*sizeof(Value));
        ref.refCount=1;
        for(int i=0;i<ref.varCount;i++){
            ref.var[i]=makeValue(vm,CLASS_INT);
            ref.var[i].num=0;
        }
        val.refID=makeRef(vm,ref);
    }else{
        val.refID=-1;
    }
    exeInit(vm,class,val);
    if(classd.initID>=0){
        LIST_ADD(vm->stack,Value,val)
        addRef(vm,val);
        exeFunc(vm,classd.methods.vals[classd.initID],0,true,true,vm->curPart);
        reduceRef(vm,vm->stack.vals[vm->stack.count-1]);
        LIST_SUB(vm->stack,Value)
    }
    return val;
}
void printClist(VM*vm,intList clist,int moduleID){
    OpcodeMsg opcode;
    Module module=vm->moduleList.vals[moduleID];
    Part part;
    Symbol symbol;
    int arg;
    for(int i=0;i<clist.count;i++){
        part=vm->partList.vals[clist.vals[i++]+module.partBasis];
        opcode=opcodeList[clist.vals[i]];
        printf("%d(%d:%d):%s",i-1,part.line,part.column,opcode.name);
        if(opcode.isArg){
            arg=clist.vals[++i];
            if(opcode.isSymbol){
                symbol=vm->symList.vals[arg+module.symBasis];
                switch(symbol.type){
                    case SYM_INT:
                        printf(" %d\n",symbol.num);
                        break;
                    case SYM_FLOAT:
                        printf(" %f\n",symbol.numf);
                        break;
                    case SYM_STRING:
                        printf(" %s\n",symbol.str);
                        break;
                    default:
                        printf(" unknown(%d)\n",symbol.type);
                        break;
                }
            }else{
                printf(" %d\n",arg);
            }
        }else{
            printf("\n");
        }
    }
}
inline static void printStack(VM*vm){
    Value val;
    char temp[20],mt[10];
    for(int i=0;i<vm->stack.count;i++){
        val=vm->stack.vals[i];
        if(val.class==CLASS_INT || val.class==CLASS_CLASS || val.class==CLASS_FUNCTION){
            sprintf(temp,"%d",val.num);
        }else if(val.class==CLASS_FLOAT){
            sprintf(temp,"%f",val.numf);
        }else if(val.class==CLASS_STRING){
            sprintf(temp,"%s",vm->refList.vals[val.refID].str);
        }else{
            strcpy(temp,"object");
        }
        if(val.refID>=0){
            sprintf(mt,"(%d)",vm->refList.vals[val.refID].varCount);
            strcat(temp,mt);
        }
        printf("stack%d:class:%s,value:%s,refID:%d\n",i,vm->classList.vals[val.class].name,temp,val.refID);
    }
}
inline static void printVar(VM*vm){
    Var var;
    char temp[20],mt[10];
    for(int i=0;i<vm->varList.count;i++){
        var=vm->varList.vals[i];
        if(var.val.class==CLASS_INT || var.val.class==CLASS_CLASS || var.val.class==CLASS_FUNCTION){
            sprintf(temp,"%d",var.val.num);
        }else if(var.val.class==CLASS_FLOAT){
            sprintf(temp,"%f",var.val.numf);
        }else if(var.val.class==CLASS_STRING){
            sprintf(temp,"%s",vm->refList.vals[var.val.refID].str);
        }else{
            strcpy(temp,"object");
        }
        if(var.val.refID>=0){
            sprintf(mt,"(%d)",vm->refList.vals[var.val.refID].varCount);
            strcat(temp,mt);
        }
        printf("variable%d(%s):class:%s,value:%s,refID:%d\n",i,var.name,vm->classList.vals[var.val.class].name,temp,var.val.refID);
    }
}
inline static void printFunc(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1];
    if(val.class!=CLASS_FUNCTION){
        reportVMError(vm,"expected an function when printing one.",curPart);
    }
    Func func=vm->funcList.vals[val.num];
    reduceRef(vm,val);
    LIST_SUB(vm->stack,Value)
    printf("function %s(",func.name);
    for(int i=0;i<func.args.count;i++){
        printf("%s",func.args.vals[i]);
        if(i<func.args.count-1){
            puts(",");
        }
    }
    printf(")module:%s:\n",vm->moduleList.vals[func.moduleID].name);
    printClist(vm,func.clist,func.moduleID);
    puts("function end\n");
}
inline static void printClass(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1];
    if(val.class!=CLASS_CLASS){
        reportVMError(vm,"expected an class when printing one.",curPart);
    }
    reduceRef(vm,val);
    LIST_SUB(vm->stack,Value)
    Class class=vm->classList.vals[val.num];
    printf("class %s:\nmembers:",class.name);
    for(int i=0;i<class.var.count;i++){
        printf("%s",class.var.vals[i]);
        if(i<class.var.count-1){
            printf(",");
        }
    }
    printf(";\n");
    Func func;
    for(int i=0;i<class.methods.count;i++){
        func=class.methods.vals[i];
        printf("method %s(",func.name);
        for(int i2=0;i2<func.args.count;i2++){
            printf("%s",func.args.vals[i2]);
            if(i2<func.args.count-1){
                puts(",");
            }
        }
        printf("):\n");
        printClist(vm,func.clist,func.moduleID);
        printf("method end\n");
    }
    puts("class end\n");
}
inline static void loadConst(VM*vm,int arg,int curPart){
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBasis];
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
    value=makeValue(vm,value.class);
    switch (symbol.type)
    {
    case SYM_INT:
        value.num=symbol.num;
        break;
    case SYM_FLOAT:
        value.numf=symbol.numf;
        break;
    case SYM_STRING:
        vm->refList.vals[value.refID].str=(char*)malloc(strlen(symbol.str)+1);
        strcpy(vm->refList.vals[value.refID].str,symbol.str);
        break;
    default:
        reportVMError(vm,"LOAD_CONST:unknown symbol type.",curPart);
        break;
    }
    LIST_ADD(vm->stack,Value,value)
}
inline static void loadVal(VM*vm,int arg,int curPart){
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBasis];
    char temp[50];
    if(symbol.type!=SYM_STRING){
        reportVMError(vm,"LOAD_VAR:symbol type must be string.",curPart);
    }
    bool isFound=false;
    if(vm->curVar>=0){
        int sum=0;
        if(searchAttr(vm,&sum,vm->varList.vals[vm->curVar].val.class,symbol.str)){
            isFound=true;
            Value val=vm->refList.vals[vm->varList.vals[vm->curVar].val.refID].var[sum];
            addRef(vm,val);
            LIST_ADD(vm->stack,Value,val)
        }
    }
    if(!isFound){
        for(int i=vm->varList.count-1;i>=0;i--){
            if(strcmp(symbol.str,vm->varList.vals[i].name)==0){
                addRef(vm,vm->varList.vals[i].val);
                LIST_ADD(vm->stack,Value,vm->varList.vals[i].val)
                isFound=true;
                break;
            }
        }
    }
    if(!isFound){
        sprintf(temp,"variable \"%s\" not found.",symbol.str);
        reportVMError(vm,temp,curPart);
    }
}
inline static void loadAttr(VM*vm,int arg,int curPart){
    checkStack(vm,1,curPart);
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBasis];
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
    int ind=0;
    if(!searchAttr(vm,&ind,a.class,symbol.str)){
        sprintf(temp,"member \"%s\" not found.",symbol.str);
        reportVMError(vm,temp,curPart);
    }
    vm->stack.vals[vm->stack.count-1]=ref.var[ind];
    addRef(vm,ref.var[ind]);
    reduceRef(vm,a);
}
inline static void loadIndex(VM*vm,int curPart){
    Value a,b;
    checkStack(vm,2,curPart);
    a=vm->stack.vals[vm->stack.count-2];
    b=vm->stack.vals[vm->stack.count-1];
    if(a.refID<0){
        reportVMError(vm,"the variable is not an array.",curPart);
    }
    if(b.class!=CLASS_INT){
        reportVMError(vm,"it must be integer when calling array.",curPart);
    }
    Ref ref=vm->refList.vals[a.refID];
    int ind=b.num+ref.varBasis;
    if((ind<0 || ind>=ref.varCount) && a.class!=CLASS_STRING){
        reportVMError(vm,"illegal array index.",curPart);
    }
    LIST_SUB(vm->stack,Value)
    if(a.class==CLASS_STRING){
        if(b.num>=strlen(vm->refList.vals[a.refID].str)){
            reportVMError(vm,"illegal string index.",curPart);
        }
        Value val=makeValue(vm,CLASS_INT);
        val.num=vm->refList.vals[a.refID].str[b.num];
        vm->stack.vals[vm->stack.count-1]=val;
    }else{
        vm->stack.vals[vm->stack.count-1]=ref.var[ind];
        addRef(vm,ref.var[ind]);
    }
    reduceRef(vm,a);
    reduceRef(vm,b);
}
inline static void storeVal(VM*vm,int arg,int curPart){
    checkStack(vm,1,curPart);
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBasis];
    if(symbol.type!=SYM_STRING){
        reportVMError(vm,"STORE_VAL:symbol type must be string.",curPart);
    }
    bool isFound=false;
    if(vm->curVar>=0){
        int sum=0;
        if(searchAttr(vm,&sum,vm->varList.vals[vm->curVar].val.class,symbol.str)){
            isFound=true;
            reduceRef(vm,vm->refList.vals[vm->varList.vals[vm->curVar].val.refID].var[sum]);
            vm->refList.vals[vm->varList.vals[vm->curVar].val.refID].var[sum]=vm->stack.vals[vm->stack.count-1];
            LIST_SUB(vm->stack,Value)
        }
    }
    if(!isFound){
        for(int i=vm->varList.count-1;i>=0;i--){
            if(strcmp(vm->varList.vals[i].name,symbol.str)==0){
                isFound=true;
                reduceRef(vm,vm->varList.vals[i].val);
                vm->varList.vals[i].val=vm->stack.vals[vm->stack.count-1];
                LIST_SUB(vm->stack,Value)
                break;
            }
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
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBasis];
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
    int ind=0;
    if(searchAttr(vm,&ind,a.class,symbol.str)){
        reduceRef(vm,ref.var[ind]);
        ref.var[ind]=vm->stack.vals[vm->stack.count-2];
        LIST_SUB(vm->stack,Value)
        LIST_SUB(vm->stack,Value)
        reduceRef(vm,a);
    }else{
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
    if(a.refID<0){
        reportVMError(vm,"the variable is not an array.",curPart);
    }
    if(b.class!=CLASS_INT){
        reportVMError(vm,"it must be integer when calling array.",curPart);
    }
    Ref ref=vm->refList.vals[a.refID];
    int ind=b.num+ref.varBasis;
    if((ind<0 || ind>=ref.varCount) && a.class!=CLASS_STRING){
        reportVMError(vm,"illegal array index.",curPart);
    }
    if(a.class==CLASS_STRING){
        if(b.num>=strlen(ref.str)){
            reportVMError(vm,"illegal string index.",curPart);
        }
        ref.str[b.num]=c.num;
    }else{
        reduceRef(vm,ref.var[ind]);
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
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBasis];
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
    char temp[100];
    if(vm->varList.count<arg){
        sprintf(temp,"in %d:it need %d variable(s) to pop but only %d in it.",vm->ptr,arg,vm->varList.count);
        reportVMError(vm,temp,curPart);
    }
    for(int i=0;i<arg;i++){
        reduceRef(vm,vm->varList.vals[vm->varList.count-1].val);
        LIST_SUB(vm->varList,Var)
    }
}
inline static void popStack(VM*vm,int arg,int curPart){
    checkStack(vm,arg,curPart);
    for(int i=0;i<arg;i++){
        reduceRef(vm,vm->stack.vals[vm->stack.count-1]);
        LIST_SUB(vm->stack,Value)
    }
}
inline static void setField(VM*vm){
    Field field;
    field.varIndex=vm->varList.count-1;
    field.funcIndex=vm->enableFunc.count-1;
    field.stackIndex=vm->stack.count-1;
    LIST_ADD(vm->fields,Field,field)
}
inline static void freeField(VM*vm,int curPart){
    Field field=vm->fields.vals[vm->fields.count-1];
    popVar(vm,vm->varList.count-field.varIndex-1,curPart);
    popStack(vm,vm->stack.count-field.stackIndex-1,curPart);
    LIST_REDUCE(vm->enableFunc,int,vm->enableFunc.count-field.funcIndex-1)
    LIST_SUB(vm->fields,Field)
}
inline static void setLoop(VM*vm){
    LIST_ADD(vm->loopList,int,vm->fields.count-1)
}
inline static void freeLoop(VM*vm,int curPart){
    int top=vm->loopList.vals[vm->loopList.count-1];
    while(vm->fields.count>top+1){
        freeField(vm,curPart);
    }
    LIST_SUB(vm->loopList,int)
}
bool searchMethod(VM*vm,Func*func,int class,char*name){
    Class classd=vm->classList.vals[class];
    for(int i=0;i<classd.methods.count;i++){
        if(strcmp(name,classd.methods.vals[i].name)==0){
            *func=classd.methods.vals[i];
            return true;
        }
    }
    for(int i=0;i<classd.parentList.count;i++){
        if(searchMethod(vm,func,classd.parentList.vals[i],name)){
            return true;
        }
    }
    return false;
}
void exeFunc(VM*vm,Func func,int argCount,bool isMethod,bool isInit,int curPart){
    checkStack(vm,argCount,curPart);
    LIST_ADD(vm->mlist,Module,vm->curModule)
    vm->curModule=vm->moduleList.vals[func.moduleID];
    Var var;
    if(isMethod){
        var.name=(char*)malloc(5);
        strcpy(var.name,"this");
        var.val=vm->stack.vals[vm->stack.count-argCount-(isInit?1:2)];
        LIST_ADD(vm->vlist,int,vm->curVar)
        vm->curVar=vm->varList.count;
        LIST_ADD(vm->varList,Var,var)
    }
    int argVarCount=0;
    for(int i=0;i<argCount && i<func.args.count;i++){
        var.val=vm->stack.vals[vm->stack.count-argCount+i];
        var.name=func.args.vals[i];
        LIST_ADD(vm->varList,Var,var)
        argVarCount++;
    }
    int retField=vm->fields.count;
    int retLoop=vm->loopList.count;
    int retStack=vm->stack.count;
    LIST_ADD(vm->funcPartList,int,curPart)
    if(!isInit){
        addFuncArgs(vm,argCount);
    }
    /*can not use popStack() because of ref*/
    LIST_REDUCE(vm->stack,Value,argCount+(isMethod?2:1)-(isInit?1:0))
    int cmdBasis=vm->curModule.cmdBasis;
    vm->curModule.cmdBasis=0;
    execute(vm,func.clist);
    vm->curModule.cmdBasis=cmdBasis;
    checkStack(vm,1,curPart);
    Value rval=vm->stack.vals[vm->stack.count-1];
    LIST_SUB(vm->stack,Value)
    LIST_SUB(vm->funcPartList,int)
    vm->curModule=vm->mlist.vals[vm->mlist.count-1];
    LIST_SUB(vm->mlist,Module)
    while(vm->loopList.count>retLoop){
        freeLoop(vm,curPart);
    }
    while(vm->fields.count>retField){
        freeField(vm,curPart);
    }
    while(vm->stack.count>retStack){
        reduceRef(vm,vm->stack.vals[vm->stack.count-1]);
        LIST_SUB(vm->stack,Value)
    }
    popVar(vm,argVarCount+(isMethod?1:0)+(isInit?0:1),curPart);
    if(isMethod){
        vm->curVar=vm->vlist.vals[vm->vlist.count-1];;
        LIST_SUB(vm->vlist,int)
    }
    LIST_ADD(vm->stack,Value,rval);
}
inline static void callFunction(VM*vm,int arg,int curPart){
    checkStack(vm,arg+1,curPart);
    char temp[50];
    Value a=vm->stack.vals[vm->stack.count-arg-1];
    if(a.class!=CLASS_STRING){
        reportVMError(vm,"CALL_FUNCTION:expected function name.",curPart);
    }
    Func func;
    func.args.vals=NULL;
    func.args.count=0;
    func.moduleID=0;
    bool isFound=false;
    if(vm->curVar>0){
        if(searchMethod(vm,&func,vm->varList.vals[vm->curVar].val.class,vm->refList.vals[a.refID].str)){
            isFound=true;
        }
    }
    if(!isFound){
        for(int i=0;i<vm->enableFunc.count;i++){
            func=vm->funcList.vals[vm->enableFunc.vals[i]];
            if(strcmp(vm->refList.vals[a.refID].str,func.name)==0){
                isFound=true;
                break;
            }
        }
    }
    if(!isFound){
        Var var;
        for(int i=0;i<vm->varList.count;i++){
            var=vm->varList.vals[i];
            if(var.val.class==CLASS_FUNCTION){
                if(strcmp(var.name,vm->refList.vals[a.refID].str)==0){
                    func=vm->funcList.vals[var.val.num];
                    isFound=true;
                    break;
                }
            }
        }
    }
    if(!isFound){
        sprintf(temp,"function \"%s\" not found.",vm->refList.vals[a.refID].str);
        reportVMError(vm,temp,curPart);
    }
    reduceRef(vm,a);
    exeFunc(vm,func,arg,false,false,curPart);
}
inline static void callMethod(VM*vm,int arg,int curPart){
    checkStack(vm,arg+2,curPart);
    char temp[50];
    Value a=vm->stack.vals[vm->stack.count-arg-2];
    Value b=vm->stack.vals[vm->stack.count-arg-1];
    if(b.class!=CLASS_STRING){
        reportVMError(vm,"CALL_FUNCTION:expected function name.",curPart);
    }
    Func func;
    if(!searchMethod(vm,&func,a.class,vm->refList.vals[b.refID].str)){
        int ind=0;
        if(searchAttr(vm,&ind,a.class,vm->refList.vals[b.refID].str)){
            Value mv=vm->refList.vals[a.refID].var[ind];
            if(mv.class!=CLASS_FUNCTION){
                printf(temp,"member %s is not a method.",vm->refList.vals[b.refID].str);
                reportVMError(vm,temp,curPart);
            }
            func=vm->funcList.vals[mv.num];
        }else{
            sprintf(temp,"method %s not found.",vm->refList.vals[b.refID].str);
            reportVMError(vm,temp,curPart);
        }
    }
    reduceRef(vm,b);
    exeFunc(vm,func,arg,true,false,curPart);
}
inline static void enableFunction(VM*vm,int arg){
    LIST_ADD(vm->enableFunc,int,arg+vm->curModule.funcBasis)
    Var var;
    var.name=vm->funcList.vals[arg+vm->curModule.funcBasis].name;
    var.val=makeValue(vm,CLASS_FUNCTION);
    var.val.num=arg+vm->curModule.funcBasis;
    LIST_ADD(vm->varList,Var,var)
}
inline static void makeObject(VM*vm,int arg){
    Value val=makeValue(vm,arg+vm->curModule.classBasis);
    LIST_ADD(vm->stack,Value,val)
}
bool searchParent(VM*vm,int class,int parent){
    Class classd=vm->classList.vals[class];
    int temp;
    for(int i=0;i<classd.parentList.count;i++){
        temp=classd.parentList.vals[i];
        if(temp==parent){
            return true;
        }else if(searchParent(vm,temp,parent)){
            return true;
        }
    }
    return false;
}
inline static void extendClass(VM*vm,int arg,int curPart){
    checkStack(vm,1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1];
    if(val.class!=CLASS_CLASS){
        reportVMError(vm,"the variable must be a class when extending class.",curPart);
    }
    int class=arg+vm->curModule.classBasis;
    if(!searchParent(vm,class,val.num)){
        vm->classList.vals[class].varBasis+=vm->classList.vals[val.num].var.count;
        LIST_ADD(vm->classList.vals[class].parentList,int,val.num);
    }
}
inline static void enableClass(VM*vm,int arg){
    Var var;
    int class=arg+vm->curModule.classBasis;
    var.name=vm->classList.vals[class].name;
    var.val=makeValue(vm,CLASS_CLASS);
    var.val.num=class;
    LIST_ADD(vm->varList,Var,var)
}
inline static void setModule(VM*vm,int arg,int curPart){
    Symbol symbol=vm->symList.vals[arg+vm->curModule.symBasis];
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
static void getVarCount(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1];
    Value v2=makeValue(vm,CLASS_INT);
    if(val.refID<0){
        v2.num=0;
    }else{
        Ref ref=vm->refList.vals[val.refID];
        if(val.class==CLASS_STRING){
            v2.num=strlen(ref.str);
        }else{
            v2.num=ref.varCount;
        }
    }
    vm->stack.vals[vm->stack.count-1]=v2;
    reduceRef(vm,val);
}
inline static void getVarClass(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1];
    Value v2=makeValue(vm,CLASS_CLASS);
    v2.num=val.class;
    reduceRef(vm,val);
    vm->stack.vals[vm->stack.count-1]=v2;
}
inline static void resizeVar(VM*vm,int curPart){
    checkStack(vm,2,curPart);
    Value a,b;
    a=vm->stack.vals[vm->stack.count-2];
    b=vm->stack.vals[vm->stack.count-1];
    if(b.class!=CLASS_INT){
        reportVMError(vm,"expected an integer to resize a variable.",curPart);
    }
    if(a.refID<0){
        reportVMError(vm,"expected memory for the variable to resize.",curPart);
    }
    int rcount=vm->refList.vals[a.refID].varCount;
    if(a.class==CLASS_STRING){
        vm->refList.vals[a.refID].str=(char*)realloc(vm->refList.vals[a.refID].str,b.num);
    }else{
        vm->refList.vals[a.refID].varCount=b.num;
        vm->refList.vals[a.refID].var=(Value*)realloc(vm->refList.vals[a.refID].var,b.num*sizeof(Value));
        Value val=makeValue(vm,CLASS_INT);
        val.num=0;
        for(int i=rcount;i<b.num;i++){
            vm->refList.vals[a.refID].var[i]=val;
        }
    }
    reduceRef(vm,a);
    reduceRef(vm,b);
    LIST_SUB(vm->stack,Value)
    LIST_SUB(vm->stack,Value)
}
inline static void makeArray(VM*vm,int arg,int curPart){
    checkStack(vm,arg,curPart);
    Ref ref;
    ref.varCount=arg;
    ref.varBasis=0;
    ref.refCount=1;
    ref.str=NULL;
    ref.var=(Value*)malloc(arg*sizeof(Value));
    for(int i=0;i<arg;i++){
        ref.var[i]=vm->stack.vals[vm->stack.count-arg+i];
    }
    LIST_REDUCE(vm->stack,Value,arg-1)
    vm->stack.vals[vm->stack.count-1]=makeValue(vm,CLASS_INT);
    vm->stack.vals[vm->stack.count-1].refID=makeRef(vm,ref);
    vm->stack.vals[vm->stack.count-1].num=0;
}
static void exeOpt(VM*vm,int ind,int curPart){
    Value a;
    char temp[50];
    a=vm->stack.vals[vm->stack.count-2];
    Class class=vm->classList.vals[a.class];
    if(class.optID[ind]<0){
        sprintf(temp,"undefined operation for class %s.",class.name);
        reportVMError(vm,temp,curPart);
    }
    exeFunc(vm,class.methods.vals[class.optID[ind]],1,true,true,curPart);
}
inline static void mexit(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1];
    if(val.class!=CLASS_INT){
        reportVMError(vm,"it must be integer when exiting.",curPart);
    }
    exit(val.num);
}
inline static void makeRange(VM*vm,int curPart){
    checkStack(vm,3,curPart);
    Value start,end,dis;
    start=vm->stack.vals[vm->stack.count-3];
    if(start.class!=CLASS_INT){
        reportVMError(vm,"the start argument of range must be integer.",curPart);
    }
    end=vm->stack.vals[vm->stack.count-2];
    if(end.class!=CLASS_INT){
        reportVMError(vm,"the end argument of range must be integer.",curPart);
    }
    dis=vm->stack.vals[vm->stack.count-1];
    if(dis.class!=CLASS_INT){
        reportVMError(vm,"the distance argument of range must be integer.",curPart);
    }
    int len=(end.num-start.num+1)/dis.num;
    Value ran=makeValue(vm,CLASS_INT);
    Ref ref;
    ref.str=NULL;
    ref.varCount=len;
    ref.varBasis=0;
    ref.refCount=1;
    ref.var=(Value*)malloc(len*sizeof(Value));
    for(int i=0;i<len;i++){
        ref.var[i]=makeValue(vm,CLASS_INT);
        ref.var[i].num=start.num+i*dis.num;
    }
    ran.refID=makeRef(vm,ref);
    reduceRef(vm,start);
    reduceRef(vm,end);
    reduceRef(vm,dis);
    LIST_SUB(vm->stack,Value)
    LIST_SUB(vm->stack,Value)
    vm->stack.vals[vm->stack.count-1]=ran;
}
inline static void copyObject(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1];
    Value v2=val;
    if(val.refID>=0){
        Ref ref,r2;
        ref=vm->refList.vals[val.refID];
        r2=ref;
        r2.refCount=1;
        r2.var=(Value*)malloc(ref.varCount*sizeof(Value));
        /*for(int i=0;i<ref.varCount;i++){
            //考虑用memcpy()代替
            r2.var[i]=ref.var[i];
        }*/
        memcpy(r2.var,ref.var,ref.varCount*sizeof(Value));
        if(val.class==CLASS_STRING && ref.str!=NULL){
            r2.str=(char*)malloc(strlen(ref.str)+1);
            strcpy(r2.str,ref.str);
        }
        v2.refID=makeRef(vm,r2);
    }
    reduceRef(vm,val);
    vm->stack.vals[vm->stack.count-1]=v2;
}
inline static void strFormat(VM*vm,int curPart){
    checkStack(vm,2,curPart);
    Value str=vm->stack.vals[vm->stack.count-2];
    Value val=vm->stack.vals[vm->stack.count-1];
    if(str.class!=CLASS_STRING){
        reportVMError(vm,"the variable must be string when formating.",curPart);
    }
    char temp[100];
    Ref ref;
    if(vm->refList.vals[str.refID].str==NULL){
        vm->refList.vals[str.refID].str=(char*)malloc(1);
        vm->refList.vals[str.refID].str[0]='\0';
    }
    switch(val.class){
        case CLASS_INT:
            sprintf(temp,"%d",val.num);
            vm->refList.vals[str.refID].str=(char*)realloc(vm->refList.vals[str.refID].str,strlen(vm->refList.vals[str.refID].str)+strlen(temp)+1);
            strcat(vm->refList.vals[str.refID].str,temp);
            break;
        case CLASS_FLOAT:
            sprintf(temp,"%f",val.numf);
            vm->refList.vals[str.refID].str=(char*)realloc(vm->refList.vals[str.refID].str,strlen(vm->refList.vals[str.refID].str)+strlen(temp)+1);
            strcat(vm->refList.vals[str.refID].str,temp);
            break;
        case CLASS_STRING:
            ref=vm->refList.vals[val.refID];
            vm->refList.vals[str.refID].str=(char*)realloc(vm->refList.vals[str.refID].str,strlen(vm->refList.vals[str.refID].str)+strlen(ref.str)+1);
            strcat(vm->refList.vals[str.refID].str,ref.str);
            break;
        default:
            sprintf(temp,"unsupport class %s to format.",vm->classList.vals[val.class].name);
            break;
    }
    popStack(vm,2,curPart);
}
inline static void mprint(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1];
    if(val.class!=CLASS_STRING){
        reportVMError(vm,"the argument must be a string when printing.",curPart);
    }
    printf("%s",vm->refList.vals[val.refID].str);
    popStack(vm,1,curPart);
}
inline static void minput(VM*vm,int curPart){
    Value val=makeValue(vm,CLASS_STRING);
    vm->refList.vals[val.refID].str=(char*)malloc(MAX_WORD_LENGTH);
    scanf("%s",vm->refList.vals[val.refID].str);
    LIST_ADD(vm->stack,Value,val)
}
inline static void getVarBasis(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1];
    Value result=makeValue(vm,CLASS_INT);
    if(val.refID>=0){
        result.num=vm->refList.vals[val.refID].varBasis;
    }else{
        result.num=0;
    }
    reduceRef(vm,val);
    vm->stack.vals[vm->stack.count-1]=result;
}
inline static void setVarBasis(VM*vm,int curPart){
    checkStack(vm,2,curPart);
    Value val,sum;
    val=vm->stack.vals[vm->stack.count-2];
    sum=vm->stack.vals[vm->stack.count-1];
    if(sum.class!=CLASS_INT){
        reportVMError(vm,"it must be integer when setting varBasis.",curPart);
    }
    if(val.refID>=0){
        vm->refList.vals[val.refID].varBasis=sum.num;
    }
    popStack(vm,2,curPart);
}
inline static void strCompare(VM*vm,int curPart){
    checkStack(vm,2,curPart);
    Value str1,str2;
    str1=vm->stack.vals[vm->stack.count-2];
    str2=vm->stack.vals[vm->stack.count-1];
    if(str1.class!=CLASS_STRING || str2.class!=CLASS_STRING){
        reportVMError(vm,"it must be string when comparing.",curPart);
    }
    Value result=makeValue(vm,CLASS_INT);
    if(strcmp(vm->refList.vals[str1.refID].str,vm->refList.vals[str2.refID].str)==0){
        result.num=true;
    }else{
        result.num=false;
    }
    reduceRef(vm,str1);
    reduceRef(vm,str2);
    LIST_SUB(vm->stack,Value)
    vm->stack.vals[vm->stack.count-1]=result;
}
inline static void mreadTextFile(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    char temp[50];
    Value val=vm->stack.vals[vm->stack.count-1];
    if(val.class!=CLASS_STRING){
        reportVMError(vm,"the file name must be string when reading text file",curPart);
    }
    FILE*fp=fopen(vm->refList.vals[val.refID].str,"rt");
    if(fp==NULL){
        sprintf(temp,"fail to read text file \"%s\".",vm->refList.vals[val.refID].str);
        reportVMError(vm,temp,curPart);
    }
    reduceRef(vm,val);
    val=makeValue(vm,CLASS_STRING);
    fseek(fp,0,SEEK_END);
    int len=ftell(fp);
    rewind(fp);
    vm->refList.vals[val.refID].str=(char*)malloc(len+1);
    len=fread(vm->refList.vals[val.refID].str,1,len,fp);
    fclose(fp);
    vm->refList.vals[val.refID].str[len]='\0';
    vm->stack.vals[vm->stack.count-1]=val;
}
inline static void mwriteTextFile(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    char temp[50];
    Value val=vm->stack.vals[vm->stack.count-2];
    Value str=vm->stack.vals[vm->stack.count-1];
    if(val.class!=CLASS_STRING){
        reportVMError(vm,"the file name must be string when reading text file",curPart);
    }
    FILE*fp=fopen(vm->refList.vals[val.refID].str,"wt");
    if(fp==NULL){
        sprintf(temp,"fail to read text file \"%s\".",vm->refList.vals[val.refID].str);
        reportVMError(vm,temp,curPart);
    }
    fwrite(vm->refList.vals[str.refID].str,1,strlen(vm->refList.vals[str.refID].str),fp);
    fclose(fp);
    reduceRef(vm,str);
    reduceRef(vm,val);
    LIST_SUB(vm->stack,Value)
    LIST_SUB(vm->stack,Value)
}
inline static void dllOpen(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    char temp[50];
    Value val=vm->stack.vals[vm->stack.count-1];
    if(val.class!=CLASS_STRING){
        reportVMError(vm,"the dll name must be string when opening a dll.",curPart);
    }
    int ind=-1;
    for(int i=0;i<vm->dllptrList.count;i++){
        if(vm->dllptrList.vals[i]==NULL){
            ind=i;
            break;
        }
    }
    if(ind<0){
        ind=vm->dllptrList.count;
        LIST_ADD(vm->dllptrList,Dllptr,NULL)
    }
    char*file=vm->refList.vals[val.refID].str;
    #ifdef LINUX
    vm->dllptrList.vals[ind]=dlopen(file,RTLD_LAZY);
    if(vm->dllptrList.vals[ind]==NULL){
        char*path=(char*)malloc(strlen(file)+strlen(vm->path)+2);
        sprintf(path,"%s/lib/%s",vm->path,file);
        vm->dllptrList.vals[ind]=dlopen(path,RTLD_LAZY);
        if(vm->dllptrList.vals[ind]==NULL){
            sprintf(temp,"opening dll \"%s\" failed.",vm->refList.vals[val.refID].str);
            reportVMError(vm,temp,curPart);
        }
        free(path);
    }
    #else
    vm->dllptrList.vals[ind]=LoadLibrary(file);
    #endif
    reduceRef(vm,val);
    val=makeValue(vm,CLASS_INT);
    val.num=ind;
    vm->stack.vals[vm->stack.count-1]=val;
}
inline static void dllClose(VM*vm,int curPart){
    checkStack(vm,1,curPart);
    Value val=vm->stack.vals[vm->stack.count-1];
    if(val.class!=CLASS_INT){
        reportVMError(vm,"the dll index must be an integer.",curPart);
    }
    if(val.num<0 || val.num>=vm->dllptrList.count){
        reportVMError(vm,"illegal dll index when closing dll.",curPart);
    }
    if(vm->dllptrList.vals[val.num]!=NULL){
        #ifdef LINUX
        dlclose(vm->dllptrList.vals[val.num]);
        #else
        FreeLibrary(vm->dllptrList.vals[val.num]);
        #endif
        vm->dllptrList.vals[val.num]=NULL;
    }
    reduceRef(vm,val);
    LIST_SUB(vm->stack,Value)
}
inline static void dllExecute(VM*vm,int curPart){
    checkStack(vm,4,curPart);
    Value val=vm->stack.vals[vm->stack.count-4];
    Value ret=vm->stack.vals[vm->stack.count-3];
    Value fname=vm->stack.vals[vm->stack.count-2];
    Value arg=vm->stack.vals[vm->stack.count-1];
    if(val.class!=CLASS_INT){
        reportVMError(vm,"the dll index must be an integer.",curPart);
    }
    if(ret.class!=CLASS_CLASS){
        reportVMError(vm,"expected a dll return type.",curPart);
    }
    if(fname.class!=CLASS_STRING){
        reportVMError(vm,"expected a dll function name.",curPart);
    }
    if(val.num<0 || val.num>=vm->dllptrList.count){
        reportVMError(vm,"illegal dll index when calling dll.",curPart);
    }
    void*dllptr=vm->dllptrList.vals[val.num];
    if(dllptr==NULL){
        reportVMError(vm,"the dll has already closed.",curPart);
    }
    char*name=vm->refList.vals[fname.refID].str;
    char*str=NULL;
    #ifdef LINUX
    #define exedll \
        dlsym(dllptr,name);\
        str=dlerror();\
        if(str!=NULL){\
            reportVMError(vm,str,curPart);\
        }
    #else
    #define exedll \
        GetProcAddress(dllptr,name);
    #endif
    if(ret.num==CLASS_INT){
        ret=makeValue(vm,CLASS_INT);
        if(arg.class==CLASS_INT){
            int (*func)(int)=exedll;
            ret.num=func(arg.num);
        }else if(arg.class==CLASS_FLOAT){
            int (*func)(float)=exedll;
            ret.num=func(arg.numf);
        }else if(arg.class==CLASS_STRING){
            int (*func)(char*)=exedll;
            ret.num=func(vm->refList.vals[arg.refID].str);
        }else{
            reportVMError(vm,"dll can only support int,float or string for argument.",curPart);
        }
    }else if(ret.num==CLASS_FLOAT){
        ret=makeValue(vm,CLASS_FLOAT);
        if(arg.class==CLASS_INT){
            float (*func)(int)=exedll;
            ret.numf=func(arg.num);
        }else if(arg.class==CLASS_FLOAT){
            float (*func)(float)=exedll;
            ret.numf=func(arg.numf);
        }else if(arg.class==CLASS_STRING){
            float (*func)(char*)=exedll;
            ret.numf=func(vm->refList.vals[arg.refID].str);
        }else{
            reportVMError(vm,"dll can only support int,float or string for argument.",curPart);
        }
    }else if(ret.num==CLASS_STRING){
        ret=makeValue(vm,CLASS_STRING);
        if(arg.class==CLASS_INT){
            char* (*func)(int)=exedll;
            vm->refList.vals[ret.refID].str=func(arg.num);
        }else if(arg.class==CLASS_FLOAT){
            char* (*func)(float)=exedll;
            vm->refList.vals[ret.refID].str=func(arg.numf);
        }else if(arg.class==CLASS_STRING){
            char* (*func)(char*)=exedll;
            vm->refList.vals[ret.refID].str=func(vm->refList.vals[arg.refID].str);
        }else{
            reportVMError(vm,"dll can only support int,float or string for argument.",curPart);
        }
    }else{
        reportVMError(vm,"dll return type can only support int,float,string.",curPart);
    }
    #undef exedll
    popStack(vm,4,curPart);
    LIST_ADD(vm->stack,Value,ret)
}
#define EXE_OPT(opt,ind) \
    checkStack(vm,2,curPart);\
    a=vm->stack.vals[vm->stack.count-2];\
    b=vm->stack.vals[vm->stack.count-1];\
    switch(a.class){\
        case CLASS_INT:\
            switch(b.class){\
                case CLASS_INT:\
                    c=makeValue(vm,CLASS_INT);\
                    c.num=a.num opt b.num;\
                    break;\
                case CLASS_FLOAT:\
                    c=makeValue(vm,CLASS_FLOAT);\
                    c.numf=a.num opt b.numf;\
                    break;\
                default:\
                    reportVMError(vm,"unsupport int operation.",curPart);\
            }\
            reduceRef(vm,a);\
            reduceRef(vm,b);\
            LIST_SUB(vm->stack,Value)\
            vm->stack.vals[vm->stack.count-1]=c;\
            break;\
        case CLASS_FLOAT:\
            c=makeValue(vm,CLASS_FLOAT);\
            switch(b.class){\
                case CLASS_INT:\
                    c.numf=a.numf opt b.num;\
                    break;\
                case CLASS_FLOAT:\
                    c.numf=a.numf opt b.numf;\
                    break;\
                default:\
                    reportVMError(vm,"unsupport float operation.",curPart);\
            }\
            reduceRef(vm,a);\
            reduceRef(vm,b);\
            LIST_SUB(vm->stack,Value)\
            vm->stack.vals[vm->stack.count-1]=c;\
            break;\
        case CLASS_CLASS:\
            if(b.class!=CLASS_CLASS){\
                reportVMError(vm,"expected a class to equal.",curPart);\
            }\
            c=makeValue(vm,CLASS_INT);\
            c.num=a.num opt b.num;\
            reduceRef(vm,a);\
            reduceRef(vm,b);\
            LIST_SUB(vm->stack,Value)\
            vm->stack.vals[vm->stack.count-1]=c;\
            break;\
        default:\
            exeOpt(vm,ind,curPart);\
    }

#define EXE_OPT_BIT(opt,ind) \
    checkStack(vm,2,curPart);\
    a=vm->stack.vals[vm->stack.count-2];\
    b=vm->stack.vals[vm->stack.count-1];\
    if(a.class==CLASS_INT){\
        if(b.class!=CLASS_INT){\
            reportVMError(vm,"expected an integer to operate.",curPart);\
        }\
        c=makeValue(vm,CLASS_INT);\
        c.num= a.num opt b.num;\
        reduceRef(vm,a);\
        reduceRef(vm,b);\
        LIST_SUB(vm->stack,Value)\
        vm->stack.vals[vm->stack.count-1]=c;\
    }else{\
        exeOpt(vm,ind,curPart);\
    }

void execute(VM*vm,intList clist){
    int opcode,curPart;
    char temp[50];
    Value a,b,c;
    for(int i=0;i<clist.count;i++){
        curPart=clist.vals[i++]+vm->curModule.partBasis;
        opcode=clist.vals[i];
        vm->ptr=i;
        //printf("cmd:%d:%s\n",i,opcodeList[opcode].name);
        vm->curPart=curPart;
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
                if(a.class==CLASS_INT){
                    a.num=-a.num;
                }else if(a.class==CLASS_FLOAT){
                    a.numf=-a.numf;
                }else{
                    reportVMError(vm,"unsupported class for prefix operation \"-\".",curPart);
                }
                vm->stack.vals[vm->stack.count-1]=a;
                break;
            case OPCODE_INVERT:
                a=vm->stack.vals[vm->stack.count-1];
                if(a.class==CLASS_INT){
                    a.num=~a.num;
                }else{
                    reportVMError(vm,"unsupported class for prefix operation \"~\".",curPart);
                }
                vm->stack.vals[vm->stack.count-1]=a;
                break;
            case OPCODE_NOT:
                a=vm->stack.vals[vm->stack.count-1];
                if(a.class==CLASS_INT){
                    a.num=!a.num;
                }else if(a.class==CLASS_FLOAT){
                    a.numf=!a.numf;
                }else{
                    reportVMError(vm,"unsupported class for prefix operation \"!\".",curPart);
                }
                vm->stack.vals[vm->stack.count-1]=a;
                break;
            case OPCODE_LOAD_CONST:
                loadConst(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_LOAD_VAR:
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
                i=clist.vals[i]+vm->curModule.cmdBasis-1;
                break;
            case OPCODE_JUMP_IF_FALSE:
                checkStack(vm,1,curPart);
                a=vm->stack.vals[vm->stack.count-1];
                if(a.class!=CLASS_INT){
                    reportVMError(vm,"expected an integer in condition statement.",curPart);
                }
                i++;
                if(!a.num){
                    i=clist.vals[i]+vm->curModule.cmdBasis-1;
                }
                reduceRef(vm,a);
                LIST_SUB(vm->stack,Value)
                break;
            case OPCODE_SET_FIELD:
                setField(vm);
                break;
            case OPCODE_FREE_FIELD:
                freeField(vm,curPart);
                break;
            case OPCODE_SET_LOOP:
                setLoop(vm);
                break;
            case OPCODE_FREE_LOOP:
                freeLoop(vm,curPart);
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
            case OPCODE_PRINT_FUNC:
                printFunc(vm,curPart);
                break;
            case OPCODE_PRINT_CLASS:
                printClass(vm,curPart);
                break;
            case OPCODE_GET_VARCOUNT:
                getVarCount(vm,curPart);
                break;
            case OPCODE_RESIZE_VAR:
                resizeVar(vm,curPart);
                break;
            case OPCODE_MAKE_ARRAY:
                makeArray(vm,clist.vals[++i],curPart);
                break;
            case OPCODE_GET_CLASS:
                getVarClass(vm,curPart);
                break;
            case OPCODE_EXIT:
                mexit(vm,curPart);
                break;
            case OPCODE_MAKE_RANGE:
                makeRange(vm,curPart);
                break;
            case OPCODE_COPY_OBJECT:
                copyObject(vm,curPart);
                break;
            case OPCODE_STR_FORMAT:
                strFormat(vm,curPart);
                break;
            case OPCODE_PRINT:
                mprint(vm,curPart);
                break;
            case OPCODE_INPUT:
                minput(vm,curPart);
                break;
            case OPCODE_GET_VARBASIS:
                getVarBasis(vm,curPart);
                break;
            case OPCODE_SET_VARBASIS:
                setVarBasis(vm,curPart);
                break;
            case OPCODE_STR_COMPARE:
                strCompare(vm,curPart);
                break;
            case OPCODE_READ_TEXT_FILE:
                mreadTextFile(vm,curPart);
                break;
            case OPCODE_WRITE_TEXT_FILE:
                mwriteTextFile(vm,curPart);
                break;
            case OPCODE_DLL_OPEN:
                dllOpen(vm,curPart);
                break;
            case OPCODE_DLL_CLOSE:
                dllClose(vm,curPart);
                break;
            case OPCODE_DLL_EXECUTE:
                dllExecute(vm,curPart);
                break;
            default:
                sprintf(temp,"unknown operation code (%d).",opcode);
                reportVMError(vm,temp,curPart);
        }
    }
}