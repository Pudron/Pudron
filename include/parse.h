#ifndef _PD_PARSE_H_
#define _PD_PARSE_H_
#include"normal.h"
typedef struct{
    int power;
    ReturnType rtype;
    HandleType handle_prefix;
    HandleType handle_infix;
    HandleType handle_postfix;
}Operat;
LIST_DECLARE(Operat)
Token nextToken(Parser*parser);
/*getExpression():将结果储存到AX*/
bool getExpression(Parser*parser,CmdList*clist,ReturnType*rtype,Environment envirn);
bool getVariableDef(Parser*parser,VariableList*vlist,CmdList*clist,bool isPart,int*partSize,Environment envirn);
bool getAssignment(Parser*parser,CmdList*clist,Environment envirn);/*赋值*/
/*getVarRef():获取一个变量，将其地址存到AX中*/
bool getVarRef(Parser*parser,char*varName,CmdList*clist,Variable*var,Environment envirn);
void getBlock(Parser*parser,CmdList*clist,Environment envirn);
bool getConditionState(Parser*parser,CmdList*clist,Environment envirn);
bool getWhileLoop(Parser*parser,CmdList*clist,Environment envirn);
/*getInsideSub():获得内建方法*/
bool getInsideSub(Parser*parser,CmdList*clist,Environment envirn);
/*getArray():先将要赋值的首地址存在AX中*/
bool getArray(Parser*parser,CmdList*clist,Variable var,Environment envirn);
/*getFunctionDef():返回值存在AX中*/
bool getFunctionDef(Parser*parser,FunctionList*funcList);
#endif