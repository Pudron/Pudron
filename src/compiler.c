#include"compiler.h"
Parser compile(Parser*parent,char*fileName,bool isLib){
    Parser parser;
    initParser(&parser,(parent==NULL)?true:false);
    if(!readTextFile(&parser.code,fileName)){
        exit(-1);
    }
    parser.fileName=fileName;
    parser.isLib=isLib;
    if(parent!=NULL){
        parser.partList=parent->partList;
        parser.clist=parent->clist;
        parser.symList=parent->symList;
        parser.funcList=parent->funcList;
        parser.classList=parent->classList;
        parser.moduleList=parent->moduleList;
    }
    int len=strlen(fileName);
    char c;
    char*name2=(char*)malloc(len-strlen(FILE_POSTFIX)+1);
    for(int i=0;i<len;i++){
        c=fileName[i];
        if(c=='.'){
            name2[i]='\0';
            break;
        }
        name2[i]=c;
    }
    Module module={name2,0,0,0,0,0};
    if(!isLib){
        module.classBase=STD_CLASS_COUNT;
    }
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
void run(char*fileName,bool isLib){
    Parser parser=compile(NULL,fileName,isLib);
    if(isLib){
        export(parser);
        puts("done\n");
    }else{
        VM vm;
        initVM(&vm,parser);
        execute(&vm,vm.clist);
    }
}
#ifndef RELEASE
void test(char*fileName,bool isLib){
    Parser parser=compile(NULL,fileName,isLib);
    char text[1000];
    classToString(parser,text);
    printf("%s",text);
    funcToString(parser,parser.funcList,text);
    printf("%s",text);
    clistToString(parser,parser.clist,text,parser.moduleList.vals[0]);
    printf("clist(size:%d):\n%s\n",parser.clist.count,text);
    if(isLib){
        export(parser);
    }else{
        VM vm;
        initVM(&vm,parser);
        execute(&vm,vm.clist);
    }
}
#endif