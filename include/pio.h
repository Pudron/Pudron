#ifndef _PD_PIO_H_
#define _PD_PIO_H_
#include"common.h"
typedef struct{
    char*dat;
    int count;
    int ptr;
}Bin;
char*readTextFile(char*fileName);
/*void export(Parser parser,char*outputName);
void import(Parser*parser,char*fileName);*/
#endif