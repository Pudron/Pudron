#ifndef _PD_EXEC_H_
#define _PD_EXEC_H_
#include"normal.h"
typedef struct{
    int reg[REG_COUNT];
    intList stack;/*栈*/
    int*data;/*数据区*/
    int dataSize;
    int ptr;/*执行指针*/
    CmdList exeClist;
    int null;
}VM;/*虚拟机*/
void initVM(VM*vm,Parser parser);
void execute(VM*vm);
void dataToString(char*text,VM vm);
#endif