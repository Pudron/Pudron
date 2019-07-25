#include"compiler.h"
void compile(Parser*parser){
    Token token;
    int rline,rptr;
    int rclass;
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

        }else if(getExpression(parser,&parser->exeClist,&rclass,envirn)){
            token=nextToken(parser);
            if(token.type!=TOKEN_SEMI){
                reportError(parser,"expected \";\" after an expression");
            }
            addCmd1(&parser->exeClist,HANDLE_SFREE,DATA_INTEGER,1);
        }else{
            reportError(parser,"unknown expression.");
        }
    }
    addCmd1(&parser->exeClist,HANDLE_NOP,DATA_INTEGER,0);
}