#ifndef _PD_COMPILER_H_
#define _PD_COMPILER_H_
#include"parse.h"
#include"exec.h"
void compile(Parser*parser);
#ifndef RELEASE
void test(Parser*parser);
#endif
#endif