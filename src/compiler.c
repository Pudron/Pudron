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
    Module module={fileName,0,0,0,3,0};//change 1 to 0
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
#ifndef RELEASE
void test(char*fileName){
    Parser parser=compile(NULL,fileName,true);
    char text[1000];
    classToString(parser,text);
    printf("%s",text);
    funcToString(parser,parser.funcList,text);
    printf("%s",text);
    clistToString(parser,parser.clist,text,parser.moduleList.vals[0]);
    printf("clist(size:%d):\n%s\n",parser.clist.count,text);
    VM vm;
    initVM(&vm,parser);
    execute(&vm,vm.clist);
}
#endif