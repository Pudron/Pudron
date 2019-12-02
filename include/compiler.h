#ifndef _PD_COMPILER_H_
#define _PD_COMPILER_H_
#include"parser.h"
#include"vm.h"
#include"pio.h"
Parser compile(Parser*parent,char*fileName,bool isLib);
void run(char*fileName,bool isLib,char*outputName);
void direct(char*fileName);
#ifndef RELEASE
void test(char*fileName,bool isLib,char*outputName);
#endif
#endif