#ifndef _PD_COMPILER_H_
#define _PD_COMPILER_H_
#include"parser.h"
#include"pio.h"
#include"core.h"
typedef struct{
    Tokentype tokenType;
    Opcode opcode;
    int level;
}Operat;
/*层次关系：
*全局:Compiler
*Func,Module:Unit
*Block:Env*/
typedef struct{
    Class*classDef;
    intList*breakList;
    int jumpTo;/*用于continue,-1则为无循环*/
    bool isGlobal;
}Env;
typedef struct{
    Parser parser;
    char*path;
    PdSTD pstd;
}Compiler;
int addConst(Unit*unit,Const con);
Module compileAll(char*fileName,PdSTD pstd);
#endif