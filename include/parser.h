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

#ifndef _PD_PARSER_H_
#define _PD_PARSER_H_
#include"common.h"
#include"compiler.h"
typedef struct{
    Tokentype type;
    char text[4];
    char len;
}TokenSymbol;
typedef struct{
    Tokentype type;
    char name[MAX_WORD_LENGTH];
    char other[MAX_WORD_LENGTH];
}Keyword;
Token nextToken(Parser*parser);
Token lastToken(Parser*parser);
void getAllToken(Parser*parser);
Token matchToken(Parser*parser,Tokentype et,char*str,int start);
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