#ifndef _PD_PARSER_H_
#define _PD_PARSER_H_
#include"common.h"
#include"compiler.h"
typedef struct{
    TokenType type;
    char text[4];
    char len;
}TokenSymbol;
typedef struct{
    TokenType type;
    char name[MAX_WORD_LENGTH];
    char other[MAX_WORD_LENGTH];
}Keyword;
Token nextToken(Parser*parser);
Token lastToken(Parser*parser);
void getAllToken(Parser*parser);
Token matchToken(Parser*parser,TokenType et,char*str,int start);
Parser newParser(char*fileName);
/*bool getValue(Parser*parser,intList*clist,Assign*asi,Env env);
Operat getExpression(Parser*parser,intList*clist,int level,Env env);
bool getAssignment(Parser*parser,intList*clist,Env env);
void getBlock(Parser*parser,intList*clist,Env env);
void getIfState(Parser*parser,intList*clist,Env env);
void getWhileState(Parser*parser,intList*clist,Env env);
void getFunction(Parser*parser,intList*clist,Env env);
void getClass(Parser*parser,intList*clist,Env env);
void getForState(Parser*parser,intList*clist,Env env);
void getDowhileState(Parser*parser,intList*clist,Env env);*/
#endif