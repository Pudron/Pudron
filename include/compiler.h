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
    bool isGlobal;
}Env;
typedef struct{
    Parser parser;
    char*path;
}Compiler;
Module compileAll(char*fileName,PdSTD pstd);
#endif