#include"compiler.h"
void compile(Parser*parser){
    Token token;
    int rline,rptr;
    ReturnType rtype;
    Environment envirn={NULL,NULL};
    while(1){
        rline=parser->line;
        rptr=parser->ptr;
        token=nextToken(parser);
        if(token.type==TOKEN_END){
            break;
        }
        parser->ptr=rptr;
        parser->line=rline;
        if(getAssignment(parser,&parser->exeClist,envirn)){
            
        }else if(getVariableDef(parser,&parser->varlist,&parser->exeClist,envirn)){

        }else if(getConditionState(parser,&parser->exeClist,&parser->varlist,envirn)){

        }else if(getWhileLoop(parser,&parser->exeClist,&parser->varlist,envirn)){

        }else if(getInsideSub(parser,&parser->exeClist,envirn)){

        }else if(getExpression(parser,&parser->exeClist,&rtype,envirn)){
            token=nextToken(parser);
            if(token.type!=TOKEN_SEMI){
                reportError(parser,"expected \";\" after an expression");
            }
            //addCmd1(&parser->exeClist,HANDLE_SFREE,DATA_INTEGER,1);
        }else{
            reportError(parser,"unknown expression.");
        }
    }
    addCmd1(&parser->exeClist,HANDLE_NOP,DATA_INTEGER,0);
}
#ifndef RELEASE
void test(Parser*parser){
    compile(parser);
    char text[1000];/*随时注意这里的大小*/
    clistToString(text,parser->exeClist,true);
    printf("Clist:\n%s\n",text);
    vlistToString(text,parser->varlist);
    printf("varlist:\n%s\n",text);
    VM vm;
    initVM(&vm,*parser);
    puts("Output:\n");
    execute(&vm);
    printf("\ndataSize:%d\n",vm.dataSize);
    dataToString(text,vm);
    printf("data:\n%s\n",text);
    puts("Test OK\n");
}
#endif