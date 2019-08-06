#ifndef _PD_PARSE_H_
#define _PD_PARSE_H_
#include"normal.h"
typedef struct{
    int power;
    bool isVar;
    int class;
    HandleType handle_prefix;
    HandleType handle_infix;
    HandleType handle_postfix;
}Operat;
LIST_DECLARE(Operat)
Token nextToken(Parser*parser);
bool getExpression(Parser*parser,CmdList*clist,ReturnType*rtype,Environment envirn);
bool getVariableDef(Parser*parser,VariableList*vlist,CmdList*clist,Environment envirn);
bool getAssignment(Parser*parser,CmdList*clist,Environment envirn);/*赋值*/
/*getVarRef():获取一个变量，将其地址存到AX中*/
bool getVarRef(Parser*parser,char*varName,CmdList*clist,int*class,Environment envirn);
void getBlock(Parser*parser,CmdList*clist,VariableList*vlist,Environment envirn);
bool getConditionState(Parser*parser,CmdList*clist,VariableList*vlist,Environment envirn);
bool getWhileLoop(Parser*parser,CmdList*clist,VariableList*vlist,Environment envirn);
/*getInsideSub():获得内建方法*/
bool getInsideSub(Parser*parser,CmdList*clist,Environment envirn);
bool getArray(Parser*parser,CmdList*clist,int*rclass,intList*arrayCount,Environment envirn);
#endif