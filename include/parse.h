#ifndef _PD_PARSE_H_
#define _PD_PARSE_H_
#include"normal.h"
typedef struct{
    int power;
    HandleType handle_prefix;
    HandleType handle_infix;
    HandleType handle_postfix;
}Operat;
typedef struct{
    CmdList clist;
    Cmd asCmd;
}AsCmds;
LIST_DECLARE(Operat)
LIST_DECLARE(AsCmds)
Token nextToken(Parser*parser);
bool getExpression(Parser*parser,CmdList*clist,Environment envirn);
bool getVariableDef(Parser*parser,VariableList*vlist,CmdList*clist,Environment envirn);
bool getAssignment(Parser*parser,CmdList*clist,Environment envirn);/*赋值*/
bool getVarRef(Parser*parser,char*varName,CmdList*clist,Cmd*asCmd/*赋值指令*/,Environment envirn);
#endif