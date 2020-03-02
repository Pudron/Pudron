#include"pio.h"
char*readTextFile(char*fileName){
    FILE*fp=fopen(fileName,"r");
    if(fp==NULL){
        //wprintf(L"error:can not open the file \"%s\"!\n",fileName);
        return NULL;
    }
    fseek(fp,0,SEEK_END);
    int len=ftell(fp);
    rewind(fp);
    char*text=(char*)malloc(len+1);
    len=fread(text,1,len,fp);
    fclose(fp);
    text[len]='\0';
    return text;
}
bool writeTextFile(char*fileName,char*text){
    FILE*fp=fopen(fileName,"wt");
    if(fp==NULL){
        return false;
    }
    fwrite(text,sizeof(char),strlen(text),fp);
    fclose(fp);
    return true;
}
/*当结构体元素顺序改变时，相应的顺序也要改变*/
void writeInt(charList*dat,int num){
    for(int i=0;i<sizeof(int);i++){
        LIST_ADD((*dat),char,num>>((sizeof(int)-i-1)*8))
    } 
}
int readInt(Bin*bin){
    int num=0;
    for(int i=0;i<sizeof(int);i++){
        num|=(bin->dat[bin->ptr++]&0xFF)<<((sizeof(int)-i-1)*8);
    }
    return num;
}
void writeDouble(charList*dat,double numd){
    void*temp=&numd;
    unsigned int num=*((unsigned int*)temp);
    //unsigned int num=((unsigned int*)&numd)[0];
    writeInt(dat,num);
}
double readDouble(Bin*bin){
    double numd;
    int num=readInt(bin);
    void*temp=&num;
    numd=*((double*)temp);
    //numd=((double*)&num)[0];
    return numd;
}
/*不写入最后的'\0'*/
void writeString(charList*dat,char*text){
    if(text==NULL){
        writeInt(dat,-1);
        return;
    }
    int len=strlen(text);
    writeInt(dat,len);
    for(int i=0;i<len;i++){
        LIST_ADD((*dat),char,text[i])
    }
}
char*readString(Bin*bin){
    int len=readInt(bin);
    if(len<0){
        return NULL;
    }
    char*text=(char*)memManage(NULL,len+1);
    for(int i=0;i<len;i++){
        text[i]=bin->dat[bin->ptr++];
    }
    text[len]='\0';
    return text;
}
void writeWideString(charList*dat,wchar_t*wstr){
    int len=wcslen(wstr);
    writeInt(dat,len);
    for(int i=0;i<len;i++){
        writeInt(dat,(int)wstr[i]);
    }
}
wchar_t*readWideString(Bin*bin){
    int len=readInt(bin);
    wchar_t*wstr=(wchar_t*)memManage(NULL,(len+1)*sizeof(wchar_t));
    for(int i=0;i<len;i++){
        wstr[i]=readInt(bin);
    }
    wstr[len]=L'\0';
    return wstr;
}
/*空的也写下去*/
void writeHashList(charList*dat,HashList hl){
    writeInt(dat,hl.capacity);
    for(int i=0;i<hl.capacity;i++){
        LIST_ADD((*dat),char,hl.slot[i].isUsed)
        if(hl.slot[i].isUsed){
            writeString(dat,hl.slot[i].name);
            writeInt(dat,hl.slot[i].nextSlot);
        }
    }
}
HashList readHashList(Bin*bin){
    HashList hl;
    hl.capacity=readInt(bin);
    hl.slot=(HashSlot*)memManage(NULL,hl.capacity*sizeof(HashSlot));
    HashSlot hs=HASH_NULL;
    for(int i=0;i<hl.capacity;i++){
        hl.slot[i].isUsed=bin->dat[bin->ptr++];
        if(hl.slot[i].isUsed){
            hl.slot[i].name=readString(bin);
            hl.slot[i].nextSlot=readInt(bin);
            hl.slot[i].obj=NULL;
        }else{
            hl.slot[i]=hs;
        }
    }
    return hl;
}
/*当func.exe!=NULL时返回false*/
bool writeFunc(charList*dat,Func func){
    writeString(dat,func.name);
    writeInt(dat,func.argCount);
    writeUnit(dat,getFuncUnit(func));
    if(func.exe!=NULL){
        return false;
    }
    return true;
}
Func readFunc(Bin*bin){
    Func func;
    func.exe=NULL;
    func.name=readString(bin);
    func.argCount=readInt(bin);
    setFuncUnit(&func,readUnit(bin));
    return func;
}
void writeClass(charList*dat,Class class){
    writeString(dat,class.name);
    WRITE_LIST(dat,class.parentList,Class)
    writeHashList(dat,class.memberList);
    WRITE_LIST(dat,class.varList,String)
    writeFunc(dat,class.initFunc);
}
Class readClass(Bin*bin){
    int count=0;
    Class class;
    class.name=readString(bin);
    READ_LIST(bin,class.parentList,Class,Class)
    class.memberList=readHashList(bin);
    READ_LIST(bin,class.varList,char*,String)
    class.initFunc=readFunc(bin);
    return class;
}
void writeConst(charList*dat,Const con){
    LIST_ADD((*dat),char,con.type)
    switch(con.type){
        case CONST_INT:
            writeInt(dat,con.num);
            break;
        case CONST_DOUBLE:
            writeDouble(dat,con.numd);
            break;
        case CONST_STRING:
            writeWideString(dat,con.str);
            break;
        case CONST_FUNCTION:
            writeFunc(dat,con.func);
            break;
        case CONST_CLASS:
            writeClass(dat,con.class);
            break;
        default:
            wprintf(L"export error:unknown constant type:%d.\n",con.type);
            exit(-1);
            break;
    }
}
Const readConst(Bin*bin){
    Const con;
    con.type=bin->dat[bin->ptr++];
    switch(con.type){
        case CONST_INT:
            con.num=readInt(bin);
            break;
        case CONST_DOUBLE:
            con.numd=readDouble(bin);
            break;
        case CONST_STRING:
            con.str=readWideString(bin);
            break;
        case CONST_FUNCTION:
            con.func=readFunc(bin);
            break;
        case CONST_CLASS:
            con.class=readClass(bin);
            break;
        default:
            wprintf(L"import error:unknown constant type:%d.\n",con.type);
            exit(-1);
            break;
    }
    return con;
}
void writePart(charList*dat,Part part){
    //writeString(dat,part.code);
    writeString(dat,part.fileName);
    writeInt(dat,part.line);
    writeInt(dat,part.column);
    writeInt(dat,part.start);
    writeInt(dat,part.end);
}
Part readPart(Bin*bin){
    Part part;
    part.code=NULL;
    part.fileName=readString(bin);
    part.line=readInt(bin);
    part.column=readInt(bin);
    part.start=readInt(bin);
    part.end=readInt(bin);
    return part;
}
void writeModule(charList*dat,Module mod){
    writeString(dat,mod.name);
    writeUnit(dat,getModuleUnit(mod));
}
Module readModule(Bin*bin){
    Module mod;
    mod.name=readString(bin);
    setModuleUnit(&mod,readUnit(bin));
    return mod;
}
void writeUnit(charList*dat,Unit unit){
    WRITE_LIST(dat,unit.constList,Const)
    WRITE_LIST(dat,unit.clist,Int)
    WRITE_LIST(dat,unit.mlist,Module)
    WRITE_LIST(dat,unit.plist,Part)
    WRITE_LIST(dat,unit.nlist,String)
    writeHashList(dat,unit.lvlist);
}
Unit readUnit(Bin*bin){
    int count=0;
    Unit unit;
    READ_LIST(bin,unit.constList,Const,Const)
    READ_LIST(bin,unit.clist,int,Int)
    READ_LIST(bin,unit.mlist,Module,Module)
    READ_LIST(bin,unit.plist,Part,Part)
    READ_LIST(bin,unit.nlist,char*,String)
    unit.lvlist=readHashList(bin);
    return unit;
}
void exportModule(char*fileName,Module mod){
    charList dat;
    LIST_INIT(dat)
    writeInt(&dat,FILE_SIGN);
    writeInt(&dat,VERSION);
    writeModule(&dat,mod);
    FILE*fp=fopen(fileName,"wb");
    if(fp==NULL){
        wprintf(L"export error:can not open the file \"%s\".\n",fileName);
        exit(-1);
    }
    fwrite(dat.vals,1,dat.count,fp);
    fclose(fp);
    LIST_DELETE(dat)
}
Module importModule(char*fileName){
    Bin bin;
    FILE*fp=fopen(fileName,"rb");
    if(fp==NULL){
        wprintf(L"import error:can not open the file \"%s\".\n",fileName);
        exit(-1);
    }
    fseek(fp,0,SEEK_END);
    bin.count=ftell(fp);
    rewind(fp);
    bin.ptr=0;
    bin.dat=(char*)memManage(NULL,bin.count+1);
    bin.count=fread(bin.dat,1,bin.count,fp);
    fclose(fp);
    if(bin.count<2*sizeof(int)){
        wprintf(L"import error:%s is not a Pudron Module File.",fileName);
        exit(-1);
    }
    int dat=readInt(&bin);
    if(dat!=FILE_SIGN){
        wprintf(L"import error:%s is not a Pudron Module File.",fileName);
        exit(-1);
    }
    dat=readInt(&bin);
    if(dat<VERSION_MIN){
        wprintf(L"import error:the version of the module is out of date.");
        exit(-1);
    }
    Module mod=readModule(&bin);
    free(bin.dat);
    return mod;
}