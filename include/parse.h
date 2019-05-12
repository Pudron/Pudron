#ifndef _PD_PARSE_H_
#define _PD_PARSE_H_
#include"normal.h"
typedef struct{
    int power;
    HandleType handle_prefix;
    HandleType handle_infix;
    HandleType handle_postfix;
}Operat;
LIST_DECLARE(Operat)
Token nextToken(Parser*parser);
bool getExpression(Parser*parser,CmdList*clist,Environment envirn);
bool getVariableDef(Parser*parser,VariableList*vlist,CmdList*clist,Environment envirn);
bool getAssignment(Parser*parser,CmdList*clist,Environment envirn);/*赋值*/
/*getVarRef():获取一个变量，将其地址存到AX中，用set来赋值*/
bool getVarRef(Parser*parser,char*varName,CmdList*clist,Environment envirn);
#endif