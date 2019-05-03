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

/*Types*/
#define TYPE_INTEGER 0
#define TYPE_FLOAT 1

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

#define LIST_SUB(list) \
    list.count--;

/*bool type*/
typedef enum{
    false=0,
    true
}bool;

/*Base Data*/
struct ValueBase;
union Data{
    int val;
    struct ValueBase*ptr;
};
struct ValueBase{
    int type;
    int mem;
    union Data dat;
};
typedef struct ValueBase Value;

typedef enum{
    TOKEN_END,
    TOKEN_UNKNOWN,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_WORD,
    TOKEN_VAR,
    TOKEN_FUNC,
    TOKEN_ARRAY,
    TOKEN_IF,
    TOKEN_ELIF,/*else if*/
    TOKEN_ELSE,
    TOKEN_EXCL,/*!*/
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
    DATA_FLOAT,
    DATA_REG_POINTER
}DataType;
typedef enum{
    HANDLE_NOP,
    HANDLE_MOV,
    HANDLE_ADD,
    HANDLE_SUB,
    HANDLE_SUBS,/*单目*/
    HANDLE_MUL,
    HANDLE_EQUAL,
    HANDLE_DIV,
    HANDLE_JMP,/*跳转*/
    HANDLE_PUSH,/*栈*/
    HANDLE_POP,
    HANDLE_FAC/*阶乘*/
}HandleType;
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
}Cmd;
typedef struct{
    int count;
    Cmd*vals;
}CmdList;
typedef struct{
    char name[WORD_MAX];
    int size;/*整数的数量*/
}ClassType;
typedef struct{
    int count;
    ClassType*vals;
}ClassList;
typedef struct{
    char name[WORD_MAX];
    Value value;
}Variable;
typedef struct{
    int count;
    Variable*vals;
}VariableList;
typedef struct{
    VariableList*globalVarlist;
}Environment;
typedef struct{
    char*fileName;
    char*code;
    int ptr;
    int line;
    ClassList classList;
    VariableList varlist;
}Parser;
void clistToString(char*text,CmdList clist);
void initParser(Parser*parser);
void reportError(Parser*parser,char*msg);
void reportWarning(Parser*parser,char*msg);
void connectCmdList(CmdList*clist,CmdList newClist);
#endif