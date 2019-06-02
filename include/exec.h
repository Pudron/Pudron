#ifndef _PD_EXEC_H_
#define _PD_EXEC_H_
#include"normal.h"
#include"parse.h"
void compile(Parser*parser);
void execute(Parser*parser,CmdList clist);
#endif