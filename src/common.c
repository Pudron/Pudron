#include"common.h"
/*严格按照索引顺序排列*/
const OpcodeMsg opcodeList[]={
    {OPCODE_NOP,"NOP",0},
    {OPCODE_ADD,"ADD",0},
    {OPCODE_SUB,"SUB",0},
    {OPCODE_MUL,"MUL",0},
    {OPCODE_DIV,"DIV",0},
    {OPCODE_AND,"AND",0},
    {OPCODE_OR,"OR",0},
    {OPCODE_CAND,"CAND",0},
    {OPCODE_COR,"COR",0},
    {OPCODE_LEFT,"LEFT",0},
    {OPCODE_RIGHT,"RIGHT",0},
    {OPCODE_EQUAL,"EQUAL",0},
    {OPCODE_GTHAN,"GTHAN",0},
    {OPCODE_LTHAN,"LTHAN",0},
    {OPCODE_NOT_EQUAL,"NOT_EQUAL",0},
    {OPCODE_GTHAN_EQUAL,"GTHAN_EQUAL",0},
    {OPCODE_LTHAN_EQUAL,"LTHAN_EQUAL",0},
    {OPCODE_REM,"REM",0},
    {OPCODE_INVERT,"INVERT",0},
    {OPCODE_NOT,"NOT",0},
    {OPCODE_SUBS,"SUBS",0},
    {OPCODE_LOAD_CONST,"LOAD_CONST",1},
    {OPCODE_LOAD_VAR,"LOAD_VAR",1},
    {OPCODE_LOAD_MEMBER,"LOAD_MEMBER",1},
    {OPCODE_LOAD_SUBSCRIPT,"LOAD_SUBSCRIPT",1},
    {OPCODE_STACK_COPY,"STACK_COPY",1},
    {OPCODE_POP_STACK,"POP_STACK",1},
    {OPCODE_JUMP,"JUMP",1},
    {OPCODE_JUMP_IF_FALSE,"JUMP_IF_FALSE",1},
    {OPCODE_CALL_FUNCTION,"CALL_FUNCTION",1},
    {OPCODE_RETURN,"RETURN",0},
    {OPCODE_INVERT_ORDER,"INVERT_ORDER",1},
    {OPCODE_SET_ASSIGN_COUNT,"SET_ASSIGN_COUNT",1},
    {OPCODE_ASSIGN,"ASSIGN",1},
    {OPCODE_MAKE_ARRAY,"MAKE_ARRAY",1},
    {OPCODE_GET_FOR_INDEX,"GET_FOR_INDEX",1},
    {OPCODE_LOAD_METHOD,"LOAD_METHOD",1},
    {OPCODE_CALL_METHOD,"CALL_METHOD",1},
    {OPCODE_CLASS_EXTEND,"CLASS_EXTEND",1},
    {OPCODE_LOAD_MODULE,"LOAD_MODULE",1}
};
void*memManage(void*ptr,size_t size){
    void*p=realloc(ptr,size);
    if(p==NULL){
        fprintf(stderr,"memory error.");
        exit(-1);
    }
    return p;
}
int pow2(int num){
	num+=(num==0);
	num--;
	num|=num>>1;
	num|=num>>2;
	num|=num>>4;
	num|=num>>8;
	num|=num>>16;
	num++;
	return num;
}
char*cutText(char*text,int start,int end){
    char*str;
    int len=end-start;
    if(len<=0 || text==NULL){
        return NULL;
    }
    str=(char*)malloc(len+1);
    for(int i=0;i<len;i++){
        str[i]=text[i+start];
    }
    str[len]='\0';
    return str;
}
char*cutPostfix(char*text){
    char t[MAX_WORD_LENGTH];
    int len=strlen(text);
    for(int i=0;i<len;i++){
        if(text[i]=='.'){
            t[i]='\0';
            break;
        }
        t[i]=text[i];
    }
    char*str=(char*)malloc(strlen(t)+1);
    strcpy(str,t);
    return str;
}
char*getPostfix(char*text){
    char*str=malloc(1);
    int len=strlen(text);
    int len2=0;
    int start=-1;
    for(int i=0;i<len;i++){
        if(text[i]=='.'){
            free(str);
            start=i;
            len2=len-start;
            str=(char*)malloc(len2+1);
            str[0]='.';
        }else if(start>=0){
            str[i-start]=text[i];
        }
    }
    str[len2]='\0';
    return str;
}
char*cutPath(char*text){
    int len=strlen(text);
    int start=0;
    char*str;
    for(int i=len-1;i>=0;i--){
        if(text[i]=='/' || text[i]=='\\'){
            start=i+1;
            break;
        }
    }
    int len2=len-start;
    str=(char*)malloc(len2+1);
    for(int i=0;i<len2;i++){
        str[i]=text[start+i];
    }
    str[len2]='\0';
    return str;
}
char*getPath(char*text){
    int len=strlen(text);
    int end=len-1;
    for(int i=len-1;i>=0;i--){
        if(text[i]=='/' || text[i]=='\\'){
            end=i-1;
            break;
        }
    }
    char*path=(char*)malloc(end+2);
    for(int i=0;i<=end;i++){
        path[i]=text[i];
    }
    path[end+1]='\0';
    return path;
}
void reportMsg(Msg msg){
    char*text,temp[10];
    text=cutText(msg.code,msg.start,msg.end);
    if(msg.type==MSG_ERROR){
        strcpy(temp,"error");
    }else{
        strcpy(temp,"warning");
    }
    if(text!=NULL){
        printf("%s:%d:%d:  %s:\n     %s\n%s\n",msg.fileName,msg.line,msg.column,temp,text,msg.text);
        free(text);
    }else{
        printf("%s:%d:%d:  %s:%s\n",msg.fileName,msg.line,msg.column,temp,msg.text);
    }
    if(msg.type==MSG_ERROR){
        exit(1);
    }
}
/*fnv-1a算法*/
unsigned int hashString(char*str){
    int len=strlen(str);
    unsigned int hashCode=2166136261;
    int idx=0;
    while(idx<len){
        hashCode^=str[idx];
        hashCode*=16777619;
        idx++;
    }
    return hashCode;
}
Unit newUnit(){
    Unit unit;
    LIST_INIT(unit.constList)
    LIST_INIT(unit.clist)
    LIST_INIT(unit.mlist)
    LIST_INIT(unit.plist)
    LIST_INIT(unit.nlist)
    unit.lvlist=newHashList();
    unit.curPart=-1;
    unit.ptr=0;
    return unit;
}
void setModuleUnit(Module*mod,Unit unit){
    mod->constList=unit.constList;
    mod->clist=unit.clist;
    mod->moduleList=unit.mlist;
    mod->partList=unit.plist;
    mod->lvlist=unit.lvlist;
    mod->nlist=unit.nlist;
}
void setFuncUnit(Func*func,Unit unit){
    func->constList=unit.constList;
    func->clist=unit.clist;
    func->moduleList=unit.mlist;
    func->partList=unit.plist;
    func->lvlist=unit.lvlist;
    func->nlist=unit.nlist;
}
Unit getModuleUnit(Module mod){
    Unit unit;
    unit.constList=mod.constList;
    unit.clist=mod.clist;
    unit.mlist=mod.moduleList;
    unit.plist=mod.partList;
    unit.lvlist=mod.lvlist;
    unit.nlist=mod.nlist;
    return unit;
}
Unit getFuncUnit(Func func){
    Unit unit;
    unit.constList=func.constList;
    unit.clist=func.clist;
    unit.mlist=func.moduleList;
    unit.plist=func.partList;
    unit.lvlist=func.lvlist;
    unit.nlist=func.nlist;
    return unit;
}
void printBlanks(int count){
    while(count--){
        printf("    ");
    }
}
void printConstMsg(Const con,int blankCount){
    int i;
    switch(con.type){
        case CONST_INT:
            printf("(%d)",con.num);
            break;
        case CONST_DOUBLE:
            printf("(%lf)",con.numd);
            break;
        case CONST_STRING:
            printf("(\"%s\")",con.str);
            break;
        case CONST_CLASS:
            printf("(class %s",con.class.name);
            /*if(con.class.parentList.count>1){
                printf(":");
                for(int i=0;i<con.class.parentList.count;i++){
                    printf("%s",con.class.parentList.vals[i].name);
                    if(i!=con.class.parentList.count-1){
                        printf(",");
                    }
                }
            }*/
            printf("{");
            for(int i=0;i<con.class.varList.count;i++){
                printf(con.class.varList.vals[i]);
                if(i!=con.class.varList.count-1){
                    printf(",");
                }
            }
            printf("}initFunc{\n");
            printCmds(getFuncUnit(con.class.initFunc),blankCount+1);
            printBlanks(blankCount);
            printf("})");
            break;
        case CONST_FUNCTION:
            printf("(function %s(",con.func.name);
            for(i=0;i<con.func.argCount;i++){
                printf(con.func.nlist.vals[i]);
                if(i!=con.func.argCount-1){
                    printf(",");
                }
            }
            printf("){\n");
            printCmds(getFuncUnit(con.func),blankCount+1);
            printBlanks(blankCount);
            printf("})");
            break;
        default:
            printf("(others)");
            break;
    }
}
void printCmds(Unit unit,int blankCount){
    Part part;
    int c;
    OpcodeMsg opm;
    for(int i=0;i<unit.clist.count;i++){
        printBlanks(blankCount);
        c=unit.clist.vals[i];
        if(c>=0){
            part=unit.plist.vals[c];
            opm=opcodeList[unit.clist.vals[++i]];
            printf("%d(%d:%d):%s",i-1,part.line,part.column,opm.name);
        }else{
            opm=opcodeList[unit.clist.vals[++i]];
            printf("%d:%s",i-1,opm.name);
        }
        if(opm.argCount>0){
            c=unit.clist.vals[++i];
            printf(" %d",c);
            switch(opm.opcode){
                case OPCODE_LOAD_CONST:
                    printConstMsg(unit.constList.vals[c],blankCount);
                    break;
                case OPCODE_LOAD_VAR:
                case OPCODE_LOAD_MEMBER:
                case OPCODE_LOAD_METHOD:
                case OPCODE_CLASS_EXTEND:
                case OPCODE_GET_FOR_INDEX:
                    printf("(%s)",unit.nlist.vals[c]);
                    break;
                case OPCODE_ASSIGN:
                    if(c<0){
                        printf("(EQUAL)");
                    }else{
                        printf("(%s)",opcodeList[c].name);
                    }
                    break;
                case OPCODE_LOAD_MODULE:{
                    Unit munit=getModuleUnit(unit.mlist.vals[c]);
                    printf("(Module %s{\n",unit.mlist.vals[c].name);
                    printCmds(munit,blankCount+1);
                    printBlanks(blankCount);
                    printf("})\n");
                    break;
                }
                default:
                    break;
            }
            for(int i2=1;i2<opm.argCount;i2++){
                printf(",%d",unit.clist.vals[++i]);
            }
        }
        printf("\n");
    }
}
void expandHashList(HashList*hl,int size){
    HashSlot hs;
    hs.name=NULL;
    hs.isUsed=false;
    hs.nextSlot=-1;
    hs.obj=NULL;
    size=pow2(size);
    HashList newhl;
    newhl.slot=(HashSlot*)memManage(NULL,size*sizeof(HashSlot));
    newhl.capacity=0;
    while(newhl.capacity<size){
        newhl.slot[newhl.capacity++]=hs;
    }
    /*重新插入*/
    for(int i=0;i<hl->capacity;i++){
        if(hl->slot[i].isUsed){
            hashGet(&newhl,hl->slot[i].name,hl->slot[i].obj,true);
        }
    }
    free(hl->slot);
    *hl=newhl;
}
HashList newHashList(){
    HashList hl;
    hl.capacity=1;
    hl.slot=(HashSlot*)memManage(NULL,sizeof(HashSlot));
    hl.slot[0]=(HashSlot){NULL,-1,false,NULL};
    return hl;
}
int hashGet(HashList*hl,char*name,Object*obj,bool isAdd){
    int c=hashString(name)%hl->capacity;
    int x;
    if(!hl->slot[c].isUsed){
        if(!isAdd){
            return -1;
        }
        hl->slot[c].isUsed=true;
        hl->slot[c].name=name;
        if(obj!=NULL){
            hl->slot[c].obj=obj;
        }
        return c;
    }
    while(strcmp(hl->slot[c].name,name)!=0){
        x=c;
        c=hl->slot[c].nextSlot;
        if(c<0){
            if(!isAdd){
                return -1;
            }
            for(int i=0;i<hl->capacity;i++){
                if(!hl->slot[i].isUsed){
                    hl->slot[i].isUsed=true;
                    hl->slot[i].name=name;
                    if(obj!=NULL){
                        hl->slot[i].obj=obj;
                    }
                    hl->slot[x].nextSlot=i;
                    return i;
                }
            }
            expandHashList(hl,hl->capacity+1);
            return hashGet(hl,name,obj,true);
        }
    }
    if(obj!=NULL){
        hl->slot[c].obj=obj;
    }
    return c;
}
HashList hashCopy(HashList hl){
    HashList hl2=newHashList();
    hl2.capacity=hl.capacity;
    hl2.slot=(HashSlot*)memManage(NULL,hl2.capacity*sizeof(HashSlot));
    memcpy(hl2.slot,hl.slot,hl.capacity*sizeof(HashSlot));
    return hl2;
}
HashList hashMerge(HashList hl1,HashList hl2){
    HashList hl=hashCopy(hl1);
    int index;
    for(int i=0;i<hl2.capacity;i++){
        if(hl2.slot[i].isUsed){
            index=hashGet(&hl,hl2.slot[i].name,NULL,true);
            hl.slot[index].obj=hl2.slot[i].obj;/*强制赋值*/
        }
    }
    return hl;
}
void hashPrint(HashList hl){
    printf("HashList(capacity:%d):\n",hl.capacity);
    for(int i=0;i<hl.capacity;i++){
        if(hl.slot[i].isUsed){
            printf("%d.name:%s,nextSlot:%d,",i,hl.slot[i].name,hl.slot[i].nextSlot);
        }
        if(hl.slot[i].obj==NULL){
            printf("None\n");
        }else{
            printf("Object(class:%s,refCount:%d)\n",hl.slot[i].obj->classNameList.vals[0],hl.slot[i].obj->refCount);
        }
    }
    printf("HashList End\n");
}
int addName(NameList*nlist,char*name){
    for(int i=0;i<nlist->count;i++){
        if(strcmp(nlist->vals[i],name)==0){
            return i;
        }
    }
    LIST_ADD((*nlist),Name,name)
    return nlist->count-1;
}