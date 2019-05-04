#include"exec.h"
void compile(Parser*parser){
    int rptr,rline;
    Token token;
    CmdList clist;
    Environment envirn;
    char text[1024];
    while(1){
        rptr=parser->ptr;
        rline=parser->line;
        token=nextToken(parser);
        if(token.type==TOKEN_END){
            break;
        }
        parser->ptr=rptr;
        parser->line=rline;
        LIST_INIT(clist,Cmd)
        if(getVariableDef(parser,&parser->varlist,&clist,envirn)){
            clistToString(text,clist);
            printf("Clist:\n%s\n",text);
        }else if(getAssignment(parser,&clist,envirn)){
            clistToString(text,clist);
            printf("Clist:\n%s\n",text);
        }else if(getExpression(parser,&clist,envirn)){
            token=nextToken(parser);
            if(token.type!=TOKEN_SEMI){
                reportError(parser,"expected \";\" after an expression");
            }
            clistToString(text,clist);
            printf("Clist:\n%s\n",text);
        }else{
            reportError(parser,"unknown expression.");
        }
        LIST_DELETE(clist);
    }
    vlistToString(text,parser->varlist);
    printf("VarList:\n%s\n",text);
}