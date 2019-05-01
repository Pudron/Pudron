#ifndef _PD_PARSE_H_
#define _PD_PARSE_H_
#include"normal.h"
typedef struct{
    int power;
    DataType type;
    HandleType handle_prefix;
    HandleType handle_infix;
    HandleType handle_postfix;
}Operat;
typedef struct{
    int count;
    Operat*vals;
}OperatList;
Token nextToken(Parser*parser);
bool getExpression(Parser*parser,CmdList*clist);
Msg getValueGlobalDef(Parser*parser);/*仅全局变量*/
Msg parse(Parser*parser);
#endif