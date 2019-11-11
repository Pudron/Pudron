#include"pio.h"
bool readTextFile(char**text,char*fileName){
    FILE*fp=fopen(fileName,"r");
    if(fp==NULL){
        printf("error:an not open the file \"%s\"!",fileName);
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
    for(int i=0;i<list.count;i++){\
        write##type(dat,list.vals[i]);\
    }

void writeInt(charList*dat,int num){
    for(int i=0;i<sizeof(int);i++){
        LIST_ADD((*dat),char,num>>((sizeof(int)-i-1)*8))
    } 
}
void writeFloat(charList*dat,float num){
    for(int i=0;i<sizeof(float);i++){
        LIST_ADD((*dat),char,((int)num)>>((sizeof(float)-i-1)*8))
    } 
}
void writeString(charList*dat,char*text){
    int len=strlen(text);
    writeInt(dat,len);
    for(int i=0;i<len;i++){
        LIST_ADD((*dat),char,text[i])
    }
}
void writeModule(charList*dat,Module module){
    writeString(dat,module.name);
    writeInt(dat,module.funcBase);
    writeInt(dat,module.cmdBase);
    writeInt(dat,module.symBase);
    writeInt(dat,module.classBase);
    writeInt(dat,module.partBase);
}
void writePart(charList*dat,Part part){
    writeString(dat,part.code);
    writeString(dat,part.fileName);
    writeInt(dat,part.line);
    writeInt(dat,part.column);
    writeInt(dat,part.start);
    writeInt(dat,part.end);
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
            printf("output error:unknown symbol type:%d.",symbol.type);
            exit(-1);
            break;
    }
}
void writeFunc(charList*dat,Func func){
    writeString(dat,func.name);
    WRITE_LIST(dat,func.args,String)
    WRITE_LIST(dat,func.clist,Int)
    /*without moduleID*/
}
void writeClass(charList*dat,Class class){
    writeString(dat,class.name);
    WRITE_LIST(dat,class.var,String)
    writeInt(dat,class.varBase);
    WRITE_LIST(dat,class.methods,Func)
    for(int i=0;i<OPT_METHOD_COUNT;i++){
        writeFunc(dat,class.optMethod[i]);
    }
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
    char fileNmae[MAX_WORD_LENGTH];
    int len=strlen(parser.fileNmae);
    char c;
    for(int i=0;i<len;i++){
        c=parser.fileName[i];
        if(c=='.'){
            break;
        }
        fileNmae[i]=c;
    }
    strcat(fileName,FILE_LIB_POSTFIX);
    
}