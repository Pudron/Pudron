#ifndef _PD_EXEC_H_
#define _PD_EXEC_H_
#include"normal.h"
#include"parse.h"
typedef struct{
    int reg[REG_COUNT];
    int*data;/*数据区*/
    int dataSize;
}VM;/*虚拟机*/
void compile(Parser*parser);
void execute(Parser*parser,CmdList clist);
#endif