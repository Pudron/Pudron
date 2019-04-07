/*指令格式：第一个字节为指令，第二个字节为参数格式，前两位为参数类型，后两位为参数长度*/

#ifndef _PD_NORMAL_H_
#define _PD_NORMAL_H_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define WORD_MAX 20

/*Regs*/
#define REG_AX 1
#define REG_BX 2
#define REG_CX 3/*用于小数运算*/
#define REG_DX 4

/*List Operations*/
#define LIST_INIT(list,type) \
        list.count=0;\
        list.vals=(type*)malloc(sizeof(type));

#define LIST_DELETE(list) \
        free(list.vals);\
        list.vals=NULL;\
        list.count=0;

#define LIST_ADD(list,type,val) \
        list.count++;\
        list.vals=(type *)realloc(list.vals,sizeof(type)*list.count);\
        list.vals[list.count-1]=val;

#define MSG_CHECK(msg,msg2) \
        if(msg2.type==MSG_ERROR){\
            msg.type=MSG_ERROR;\
            strcat(msg.text,msg2.text);\
        }else if(msg2.type==MSG_ERROR_MUST){\
            msg.type=MSG_ERROR_MUST;\
            strcat(msg.text,msg2.text);\
            return msg;\
        }

typedef enum{
    MSG_SUCCESS,
    MSG_NONE,
    MSG_ERROR,
    MSG_ERROR_MUST/*此类错误必须终止编译*/
}MsgType;
typedef enum{
    TOKEN_END,
    TOKEN_UNKNOWN,
    TOKEN_NUMBER,
    TOKEN_WORD,
    TOKEN_CHAR,
    TOKEN_NUM,/*关键字num*/
    TOKEN_ARRAY,
    TOKEN_STATIC,
    TOKEN_ADD,
    TOKEN_SUB,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_EQUAL,
    TOKEN_COMMA,/*,*/
    TOKEN_SEMI,/*;*/
    TOKEN_PARE1,/*(*/
    TOKEN_PARE2,/*)*/
    TOKEN_BRACE1,/*{*/
    TOKEN_BRACE2,/*}*/
    TOKEN_GTHAN,/*>*/
    TOKEN_LTHAN,/*<*/
    TOKEN_GTHAN_EQUAL,/*>=*/
    TOKEN_LTHAN_EQUAL/*<=*/
}TokenType;
typedef enum{
    DATA_REG,
    DATA_POINTER,/*指针*/
    DATA_INTEGER,
    DATA_CHAR
}DataType;
typedef enum{
    HANDLE_NOP,
    HANDLE_DATA,
    HANDLE_MOV,
    HANDLE_MOVI,/*4位类型赋值*/
    HANDLE_ADD,
    HANDLE_SUB,
    HANDLE_SUBS,/*单目*/
    HANDLE_MUL,
    HANDLE_EQUAL,
    HANDLE_DIV,
    HANDLE_JMP,/*跳转*/
    HANDLE_PUSH,/*栈*/
    HANDLE_POP,
    HANDLE_PUSHI,/*4位栈操作*/
    HANDLE_POPI,
    HANDLE_ADDF,/*带小数自动运算，规则：AX和BX存整数，CX和DX存小数位，然后直接无参数运算*/
    HANDLE_SUBF,
    HANDLE_MULF,
    HANDLE_DIVF
}HandleType;
typedef enum{
    EXTYPE_NORMAL,
    EXTYPE_ARRAY,
    EXTYPE_DYNAMIC_ARRAY
}ExType;
typedef struct{
    TokenType type;
    int num;
    char dat;/*附加数据，如小数*/
    char word[WORD_MAX];
}Token;
typedef struct{
    HandleType handle;
    DataType ta,tb;
    int a,b;
    char parac;/*参数数量*/
    char isWeak;/*用于优化中间代码*/
}Cmd;
typedef struct{
    int count;
    Cmd*vals;
    int memory;/*指令总大小*/
}CmdList;/*中间代码*/
typedef struct{
	int count;
	int memory;/*已分配内存*/
	unsigned char*vals;
}Commands;/*目标代码*/
typedef struct{
    MsgType type;
    char text[100];
}Msg;
typedef struct{
    char name[WORD_MAX];
    int size;
}ClassType;
typedef struct{
    int count;
    ClassType*vals;
}ClassList;
typedef struct{
    int type;
    char name[WORD_MAX];
    int ptr;
    char isStatic;
    ExType extype;
}Value;
typedef struct{
    int count;
    Value*vals;
}ValueList;
typedef struct{
    char*fileName;
    char*code;
    int ptr;
    int line;
    Commands cmds;/*已确定指令，储存变量*/
    CmdList clist;
    ClassList classList;
    ValueList vlist;
}Parser;
int initParser(Parser*parser);
int initCommands(Commands*cmds);
int addCmd(Commands*cmds,unsigned char c);
int addCmdInt(Commands*cmds,int c);
int addCmdDats(Commands*cmds,char count,char val);
int addCmd1(Commands*cmds,Cmd cmd);
int addCmd2(Commands*cmds,Cmd cmd);
int clistToCmds(Commands*cmds,CmdList clist);
int cmdToString(char*text,Commands cmds);
int clistToString(char*text,CmdList clist);
#endif