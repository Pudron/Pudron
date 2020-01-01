#ifndef _PD_PARSER_H_
#define _PD_PARSER_H_
#include"common.h"
#include"compiler.h"
#define ORI_DEF() int rtoken;
#define ORI_ASI() \
    rtoken=parser->curToken;

#define ORI_RET() \
    parser->curToken=rtoken;

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
typedef struct{
    TokenType tokenType;
    Opcode opcode;
    int level;
}Operat;
typedef struct{
    int classDef;/*-1为无*/
    bool isClassVarDef;
    intList*breakList;
    bool isGlobal;
    bool isFuncDef;
}Env;
typedef struct{
    intList clist;
    Command acmds,gcmds,ncmds;
}Assign;
LIST_DECLARE(Assign)
Token nextToken(Parser*parser);
void getAllToken(Parser*parser);
Token matchToken(Parser*parser,TokenType et,char*str,int start);
bool getValue(Parser*parser,intList*clist,Assign*asi,Env env);
Operat getExpression(Parser*parser,intList*clist,int level,Env env);
bool getAssignment(Parser*parser,intList*clist,Env env);
void getBlock(Parser*parser,intList*clist,Env env);
void getIfState(Parser*parser,intList*clist,Env env);
void getWhileState(Parser*parser,intList*clist,Env env);
void getFunction(Parser*parser,intList*clist,Env env);
void getClass(Parser*parser,intList*clist,Env env);
void getForState(Parser*parser,intList*clist,Env env);
void getDowhileState(Parser*parser,intList*clist,Env env);
#endif