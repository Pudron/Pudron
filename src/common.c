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
    {OPCODE_LOAD_ATTR,"LOAD_ATTR",1},
    {OPCODE_LOAD_MEMBER,"LOAD_MEMBER",1},
    {OPCODE_LOAD_SUBSCRIPT,"LOAD_SUBSCRIPT",0},
    {OPCODE_STACK_COPY,"STACK_COPY",1},
    //{OPCODE_PUSH_STACK,"PUSH_STACK",1},
    {OPCODE_POP_STACK,"POP_STACK",1},
    {OPCODE_JUMP,"JUMP",1},
    {OPCODE_JUMP_IF_FALSE,"JUMP_IF_FALSE",1},
    {OPCODE_LOAD_FIELD,"LOAD_FIELD",1},
    {OPCODE_FREE_FIELD,"FREE_FIELD",1},
    {OPCODE_SET_LOOP,"SET_LOOP",0},
    {OPCODE_FREE_LOOP,"FREE_LOOP",0},
    {OPCODE_CALL_FUNCTION,"CALL_FUNCTION",1},
    //{OPCODE_CALL_METHOD,"CALL_METHOD",true,false},
    {OPCODE_RETURN,"RETURN",0},
    {OPCODE_INVERT_ORDER,"INVERT_ORDER",1},
    //{OPCODE_ASSIGN_LEFT,"ASSIGN_LEFT",1},
    //{OPCODE_ASSIGN_RIGHT,"ASSIGN_RIGHT",1},
    {OPCODE_SET_ASSIGN_COUNT,"SET_ASSIGN_COUNT",1},
    {OPCODE_ASSIGN,"ASSIGN",1},
    {OPCODE_MAKE_ARRAY,"MAKE_ARRAY",1},
    {OPCODE_GET_FOR_INDEX,"GET_FOR_INDEX",0},
    {OPCODE_LOAD_STACK,"LOAD_STACK",0},
    {OPCODE_LOAD_ARG_COUNT,"LOAD_ARG_COUNT",0}
    /*{OPCODE_MAKE_OBJECT,"MAKE_OBJECT",true,false},
    {OPCODE_EXTEND_CLASS,"EXTEND_CLASS",true,false},
    {OPCODE_ENABLE_CLASS,"ENABLE_CLASS",true,false},
    {OPCODE_SET_MODULE,"SET_MODULE",true,true},
    {OPCODE_RETURN_MODULE,"RETURN_MODULE",false,false},
    {OPCODE_PRINT_STACK,"PRINT_STACK",false,false},
    {OPCODE_PRINT_VAR,"PRINT_VAR",false,false},
    {OPCODE_PRINT_FUNC,"PRINT_FUNC",false,false},
    {OPCODE_PRINT_CLASS,"PRINT_CLASS",false,false},
    {OPCODE_GET_VARCOUNT,"GET_VARCOUNT",false,false},
    {OPCODE_RESIZE_VAR,"RESIZE_VAR",false,false},
    {OPCODE_MAKE_ARRAY,"MAKE_ARRAY",true,false},
    {OPCODE_GET_CLASS,"GET_CLASS",false,false},
    {OPCODE_GET_VARBASIS,"GET_VARBASIS",false,false},
    {OPCODE_SET_VARBASIS,"SET_VARBASIS",false,false},
    {OPCODE_EXIT,"EXIT",false,false},
    {OPCODE_MAKE_RANGE,"MAKE_RANGE",false,false},
    {OPCODE_COPY_OBJECT,"COPY_OBJECT",false,false},
    {OPCODE_STR_FORMAT,"STR_FORMAT",false,false},
    {OPCODE_PRINT,"PRINT",false,false},
    {OPCODE_INPUT,"INPUT",false,false},
    {OPCODE_STR_COMPARE,"STR_COMPARE",false,false},
    {OPCODE_READ_TEXT_FILE,"READ_TEXT_FILE",false,false},
    {OPCODE_WRITE_TEXT_FILE,"WRITE_TEXT_FILE",false,false},
    {OPCODE_DLL_OPEN,"DLL_OPEN",false,false},
    {OPCODE_DLL_CLOSE,"DLL_CLOSE",false,false},
    {OPCODE_DLL_EXECUTE,"DLL_EXECUTE",false,false}*/
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
Unit newUnit(int varStart){
    Unit unit;
    LIST_INIT(unit.constList)
    LIST_INIT(unit.clist)
    LIST_INIT(unit.mlist)
    LIST_INIT(unit.plist)
    LIST_INIT(unit.flist)
    LIST_INIT(unit.mblist)
    unit.varStart=varStart;
    unit.curPart=-1;
    unit.ptr=0;
    return unit;
}
void setModuleUnit(Module*mod,Unit unit){
    mod->constList=unit.constList;
    mod->clist=unit.clist;
    mod->moduleList=unit.mlist;
    mod->partList=unit.plist;
    mod->fieldList=unit.flist;
    mod->memberList=unit.mblist;
}
void setFuncUnit(Func*func,Unit unit){
    func->constList=unit.constList;
    func->clist=unit.clist;
    func->moduleList=unit.mlist;
    func->partList=unit.plist;
    func->fieldList=unit.flist;
    func->memberList=unit.mblist;
}
Unit getModuleUnit(Module mod){
    Unit unit;
    unit.constList=mod.constList;
    unit.clist=mod.clist;
    unit.mlist=mod.moduleList;
    unit.plist=mod.partList;
    unit.flist=mod.fieldList;
    unit.mblist=mod.memberList;
    return unit;
}
Unit getFuncUnit(Func func){
    Unit unit;
    unit.constList=func.constList;
    unit.clist=func.clist;
    unit.mlist=func.moduleList;
    unit.plist=func.partList;
    unit.flist=func.fieldList;
    unit.mblist=func.memberList;
    return unit;
}
void getConstMsg(char*text,Const con){
    int i;
    switch(con.type){
        case CONST_INT:
            sprintf(text,"%d",con.num);
            break;
        case CONST_DOUBLE:
            sprintf(text,"%f",con.numd);
            break;
        case CONST_STRING:
            sprintf(text,"%s",con.str);
            break;
        case CONST_CLASS:
            strcpy(text,"class{");
            for(int i=0;i<con.classd.varList.count;i++){
                strcat(text,con.classd.varList.vals[i].name);
                if(i==con.classd.varList.count-1){
                    strcat(text,"}");
                }else{
                    strcat(text,",");
                }
            }
            break;
        case CONST_FUNCTION:
            strcpy(text,"function(");
            for(i=0;i<con.func.argList.count;i++){
                strcat(text,con.func.argList.vals[i].name);
                if(i==con.func.argList.count-1){
                    strcat(text,")");
                }else{
                    strcat(text,",");
                }
            }
            break;
        default:
            strcpy(text,"others");
            break;
    }
}
void printCmds(Unit unit){
    char temp[50];
    Part part;
    int c;
    OpcodeMsg opm;
    for(int i=0;i<unit.clist.count;i++){
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
                    getConstMsg(temp,unit.constList.vals[c]);
                    printf("(%s)",temp);
                    break;
                case OPCODE_LOAD_MEMBER:
                    printf("(%s)",unit.mblist.vals[c].name);
                    break;
                case OPCODE_ASSIGN:
                    if(c<0){
                        printf("(EQUAL)");
                    }else{
                        printf("(%s)",opcodeList[c].name);
                    }
                    break;
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