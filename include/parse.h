#ifndef _PD_PARSE_H_
#define _PD_PARSE_H_
#include"normal.h"

#define TYPE_NUMBER 1
#define TYPE_CHAR 0

#define PARA_FORMAT2(ta,ma,tb,mb) (ta<<6)|(ma<<4)|(tb<<2)|mb

#define MSG_PARSE_CHECK(msg,msg2) \
        MSG_CHECK(msg,msg2);\
        if(msg.type!=MSG_NONE){\
            continue;\
        }

typedef struct{
    int power;
    HandleType handle;
    int rtype;/*运算符左类型*/
    int ltype;/*运算符右类型*/
    char isFunc;
}Operat;
typedef struct{
    int count;
    Operat*vals;
}OperatList;
Msg nextToken(Parser*parser,Token*token);
Msg getExpression(Parser*parser,CmdList*cmds,int*rtype);
Msg getValueGlobalDef(Parser*parser);/*仅全局变量*/
Msg parse(Parser*parser);
#endif