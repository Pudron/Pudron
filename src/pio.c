#include"pio.h"
bool readTextFile(char**text,char*fileName){
    FILE*fp=fopen(fileName,"r");
    if(fp==NULL){
        printf("error:can not open the file \"%s\"!\n",fileName);
        return false;
    }
    fseek(fp,0,SEEK_END);
    int len=ftell(fp);
    rewind(fp);
    *text=(char*)malloc(len+1);
    len=fread(*text,1,len,fp);
    fclose(fp);
    (*text)[len]='\0';
    return true;
}
/*当结构体元素顺序改变时，相应的顺序也要改变*/
#define WRITE_LIST(dat,list,type) \
    writeInt(dat,list.count);\
    for(int i=0;i<list.count;i++){\
        write##type(dat,list.vals[i]);\
    }

/*READ_LIST()在使用前先定义变量int count;*/
#define READ_LIST(bin,list,type,typeName) \
    count=readInt(bin);\
    for(int i=0;i<count;i++){\
        LIST_ADD(list,type,read##typeName(bin))\
    }

void writeInt(charList*dat,int num){
    for(int i=0;i<sizeof(int);i++){
        LIST_ADD((*dat),char,num>>((sizeof(int)-i-1)*8))
    } 
}
int readInt(Bin*bin){
    int num=0;
    for(int i=0;i<sizeof(int);i++){
        num|=bin->dat[bin->ptr++]<<((sizeof(int)-i-1)*8);
    }
    return num;
}
void writeFloat(charList*dat,float num){
    for(int i=0;i<sizeof(float);i++){
        LIST_ADD((*dat),char,((int)num)>>((sizeof(float)-i-1)*8))
    } 
}
float readFloat(Bin*bin){
    int num=0;
    for(int i=0;i<sizeof(float);i++){
        num|=bin->dat[bin->ptr++]<<((sizeof(float)-i-1)*8);
    }
    return (float)num;
}
void writeString(charList*dat,char*text){
    int len=strlen(text);
    writeInt(dat,len);
    for(int i=0;i<len;i++){
        LIST_ADD((*dat),char,text[i])
    }
}
char*readString(Bin*bin){
    int len=readInt(bin);
    char*text=(char*)malloc(len+1);
    for(int i=0;i<len;i++){
        text[i]=bin->dat[bin->ptr++];
    }
    text[len]='\0';
    return text;
}
void writeModule(charList*dat,Module module){
    writeString(dat,module.name);
    writeInt(dat,module.funcBase);
    writeInt(dat,module.cmdBase);
    writeInt(dat,module.symBase);
    writeInt(dat,module.classBase);
    writeInt(dat,module.partBase);
}
Module readModule(Bin*bin){
    Module module;
    module.name=readString(bin);
    module.funcBase=readInt(bin);
    module.cmdBase=readInt(bin);
    module.symBase=readInt(bin);
    module.classBase=readInt(bin);
    module.partBase=readInt(bin);
    return module;
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
void writeSymbol(charList*dat,Symbol symbol){
    writeInt(dat,symbol.type);
    switch(symbol.type){
        case SYM_INT:
            writeInt(dat,symbol.num);
            break;
        case SYM_FLOAT:
            writeFloat(dat,symbol.numf);
            break;
        case SYM_STRING:
            writeString(dat,symbol.str);
            break;
        default:
            printf("output error:unknown symbol type:%d.\n",symbol.type);
            exit(-1);
            break;
    }
}
Symbol readSymbol(Bin*bin){
    Symbol symbol;
    symbol.type=readInt(bin);
    switch(symbol.type){
        case SYM_INT:
            symbol.num=readInt(bin);
            break;
        case SYM_FLOAT:
            symbol.numf=readFloat(bin);
            break;
        case SYM_STRING:
            symbol.str=readString(bin);
            break;
        default:
            printf("import error:unknown symbol type:%d.\n",symbol.type);
            exit(-1);
            break;
    }
    return symbol;
}
void writeFunc(charList*dat,Func func){
    writeString(dat,func.name);
    WRITE_LIST(dat,func.args,String)
    WRITE_LIST(dat,func.clist,Int)
    writeInt(dat,func.moduleID);
}
Func readFunc(Bin*bin){
    Func func;
    func.name=readString(bin);
    int count;
    LIST_INIT(func.args,Name)
    LIST_INIT(func.clist,int)
    READ_LIST(bin,func.args,Name,String)
    READ_LIST(bin,func.clist,int,Int)
    func.moduleID=readInt(bin);
    return func;
}
void writeClass(charList*dat,Class class){
    writeString(dat,class.name);
    WRITE_LIST(dat,class.var,String)
    writeInt(dat,class.varBase);
    WRITE_LIST(dat,class.methods,Func)
    for(int i=0;i<OPT_METHOD_COUNT;i++){
        writeFunc(dat,class.optMethod[i]);
    }
    WRITE_LIST(dat,class.parentList,Int)
}
Class readClass(Bin*bin){
    Class class;
    int count;
    class.name=readString(bin);
    LIST_INIT(class.var,Name)
    READ_LIST(bin,class.var,Name,String)
    class.varBase=readInt(bin);
    LIST_INIT(class.methods,Func)
    READ_LIST(bin,class.methods,Func,Func)
    for(int i=0;i<OPT_METHOD_COUNT;i++){
        class.optMethod[i]=readFunc(bin);
    }
    LIST_INIT(class.parentList,int)
    READ_LIST(bin,class.parentList,int,Int)
    return class;
}
void export(Parser parser){
    charList dat;
    LIST_INIT(dat,char)
    WRITE_LIST(&dat,parser.moduleList,Module)
    WRITE_LIST(&dat,parser.partList,Part)
    WRITE_LIST(&dat,parser.clist,Int)
    WRITE_LIST(&dat,parser.symList,Symbol)
    WRITE_LIST(&dat,parser.funcList,Func)
    WRITE_LIST(&dat,parser.classList,Class)
    char fileName[MAX_WORD_LENGTH];
    int len=strlen(parser.fileName);
    char c;
    for(int i=0;i<len;i++){
        c=parser.fileName[i];
        if(c=='.'){
            fileName[i]='\0';
            break;
        }
        fileName[i]=c;
    }
    strcat(fileName,FILE_LIB_POSTFIX);
    FILE*fp=fopen(fileName,"wb");
    if(fp==NULL){
        printf("output error:can not create the file %s.\n",fileName);
        exit(-1);
    }
    fwrite(dat.vals,dat.count,1,fp);
    fclose(fp);
}
void import(Parser*parser,char*fileName){
    Bin bin;
    FILE*fp=fopen(fileName,"rb");
    if(fp==NULL){
        printf("import error:%s not found.\n",fileName);
        exit(-1);
    }
    fseek(fp,0,SEEK_END);
    bin.count=ftell(fp);
    rewind(fp);
    bin.dat=(char*)malloc(bin.count+1);
    bin.count=fread(bin.dat,bin.count,1,fp);
    bin.ptr=0;
    fclose(fp);
    int count;
    int begin=parser->moduleList.count;
    int moduleID=begin;
    READ_LIST(&bin,parser->moduleList,Module,Module)
    for(int i=begin;i<parser->moduleList.count;i++){
        parser->moduleList.vals[i].funcBase+=parser->funcList.count;
        parser->moduleList.vals[i].cmdBase+=parser->clist.count;
        parser->moduleList.vals[i].symBase+=parser->symList.count;
        parser->moduleList.vals[i].classBase+=parser->classList.count;
        parser->moduleList.vals[i].partBase+=parser->partList.count;
    }
    READ_LIST(&bin,parser->partList,Part,Part)
    READ_LIST(&bin,parser->clist,int,Int)
    READ_LIST(&bin,parser->symList,Symbol,Symbol)
    begin=parser->funcList.count;
    READ_LIST(&bin,parser->funcList,Func,Func)
    for(int i=begin;i<parser->funcList.count;i++){
        parser->funcList.vals[i].moduleID+=moduleID;
    }
    begin=parser->classList.count;
    READ_LIST(&bin,parser->classList,Class,Class)
    for(int i=begin;i<parser->classList.count;i++){
        for(int i2=0;i2<parser->classList.vals[i].methods.count;i2++){
            parser->classList.vals[i].methods.vals[i2].moduleID+=moduleID;
        }
    }
}