#ifndef _PD_COMPILER_H_
#define _PD_COMPILER_H_
#include"parser.h"
#include"vm.h"
#include"pio.h"
Parser compile(Parser*parent,char*fileName,bool isLib);
#ifndef RELEASE
void test(char*fileName);
#endif
#endif