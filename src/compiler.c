#include"compiler.h"
#ifdef LINUX
#include<unistd.h>
#else
#include<windows.h>
#endif
void initStd(Parser*parser){
    Class class;
    Part part;
    class.varBasis=0;
    class.var.count=0;
    class.methods.count=0;
    class.initID=-1;
    class.destroyID=-1;
    class.initValID=-1;
    for(int i=0;i<OPT_METHOD_COUNT;i++){
        class.optID[i]=-1;
    }
    LIST_INIT(class.parentList,int)
    LIST_ADD(class.parentList,int,CLASS_META)
    class.name=(char*)malloc(4);
    strcpy(class.name,"int");
    LIST_ADD(parser->classList,Class,class)
    class.name=(char*)malloc(5);
    strcpy(class.name,"class");
    LIST_ADD(parser->classList,Class,class)
    class.name=(char*)malloc(8);
    strcpy(class.name,"function");
    LIST_ADD(parser->classList,Class,class)
    part.fileName=parser->fileName;
    part.code=NULL;
    part.line=0;
    part.column=0;
    part.start=0;
    part.end=0;
    parser->curPart=parser->partList.count;
    LIST_ADD(parser->partList,Part,part)
    char temp[MAX_WORD_LENGTH];
    sprintf(temp,"%s/lib/meta.pdl",parser->path);
    import(parser,temp);
    sprintf(temp,"%s/lib/float.pdl",parser->path);
    import(parser,temp);
    sprintf(temp,"%s/lib/string.pdl",parser->path);
    import(parser,temp);
}
Parser compile(Parser*parent,char*fileName,bool isLib){
    Parser parser;
    initParser(&parser,(parent==NULL)?true:false);
    if(!readTextFile(&parser.code,fileName)){
        exit(-1);
    }
    parser.fileName=fileName;
    parser.isLib=isLib;
    if(parent==NULL){
        char*path=(char*)malloc(MAX_WORD_LENGTH);
        int len=-1;
        #ifdef LINUX
            len=readlink("/proc/self/exe",path,MAX_WORD_LENGTH-1);
            if(len<0){
                printf("error:failed to get pudron path.\n");
                exit(-1);
            }
            path[len]='\0';
        #else
            GetModuleFileName(NULL,path,MAX_WORD_LENGTH-1);
        #endif
        parser.path=getPath(path);
        free(path);
    }
    if(parent!=NULL){
        parser.partList=parent->partList;
        parser.clist=parent->clist;
        parser.symList=parent->symList;
        parser.funcList=parent->funcList;
        parser.classList=parent->classList;
        parser.moduleList=parent->moduleList;
        parser.path=parent->path;
    }else if(!isLib){
        initStd(&parser);
    }
    char*namet=cutPostfix(fileName);
    char*name2=cutPath(namet);
    free(namet);
    Module module={name2,0,0,0,0,0};
    parser.curModule=parser.moduleList.count;
    LIST_ADD(parser.moduleList,Module,module)
    Env env={-1,false,NULL,true,false};
    getBlock(&parser,&parser.clist,env);
    if(parent!=NULL){
        parent->partList=parser.partList;
        parent->clist=parser.clist;
        parent->symList=parser.symList;
        parent->funcList=parser.funcList;
        parent->classList=parser.classList;
    }
    return parser;
}
void run(char*fileName,bool isLib,char*outputName){
    Parser parser=compile(NULL,fileName,isLib);
    if(isLib){
        export(parser,outputName);
        puts("done\n");
    }else{
        VM vm;
        initVM(&vm,parser);
        execute(&vm,vm.clist);
        exitVM(&vm);
    }
}
void direct(char*fileName){
    Parser parser;
    initParser(&parser,true);
    parser.fileName=fileName;
    parser.isLib=false;
    initStd(&parser);
    Module module={fileName,0,0,0,0,0};
    parser.curModule=parser.moduleList.count;
    LIST_ADD(parser.moduleList,Module,module)
    import(&parser,fileName);
    VM vm;
    initVM(&vm,parser);
    execute(&vm,vm.clist);
}
#ifndef RELEASE
void test(char*fileName,bool isLib,char*outputName){
    Parser parser=compile(NULL,fileName,isLib);
    char text[5000];
    classToString(parser,text);
    printf("%s",text);
    funcToString(parser,parser.funcList,text);
    printf("%s",text);
    clistToString(parser,parser.clist,text,parser.moduleList.vals[parser.curModule]);
    printf("clist(size:%d):\n%s\n",parser.clist.count,text);
    if(isLib){
        export(parser,outputName);
    }else{
        VM vm;
        initVM(&vm,parser);
        execute(&vm,vm.clist);
        exitVM(&vm);
    }
}
#endif