#ifndef _PD_COMPILER_H_
#define _PD_COMPILER_H_
#include"parser.h"
#include"pio.h"
typedef struct{
    TokenType tokenType;
    Opcode opcode;
    int level;
}Operat;
/*层次关系：
*全局:Compiler
*Func,Module:Unit
*Block:Env*/
typedef struct{
    Class*class;
    intList*breakList;
    bool isGlobal;
    Field*field;
}Env;
typedef struct{
    Parser parser;
    VarList vlist;
}Compiler;
Module compileAll(char*fileName);
Operat compileExpression(Compiler*cp,Unit*unit,int level,bool isAssign,int msgStart,Env env);
void compileBlock(Compiler*cp,Unit*unit,Env env);
#endif