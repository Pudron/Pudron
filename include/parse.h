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
LIST_DECLARE(Operat)
Token nextToken(Parser*parser);
bool getExpression(Parser*parser,CmdList*clist,Environment envirn);
bool getVariableDef(Parser*parser,VariableList*vlist,CmdList*clist,Environment envirn);
bool getVarRef(CmdList*clist,Cmd*asCmd/*赋值指令*/,Environment envirn);
#endif