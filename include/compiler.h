/***
*   Copyright (c) 2020 Pudron
*
*   This file is part of Pudron.
*
*   Pudron is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   Pudron is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
**/

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
    char*charsetCode;/*字符编码*/
}Compiler;
int addConst(Unit*unit,Const con);
Module compileAll(char*fileName,char*path,PdSTD pstd);
#endif