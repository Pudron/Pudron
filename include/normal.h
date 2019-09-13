#ifndef _PD_NORMAL_H_
#define _PD_NORMAL_H_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define WORD_MAX 20
/*Regs*/
#define REG_COUNT 7
#define REG_NULL 0
#define REG_AX 1
#define REG_BX 2
#define REG_CX 3/*用于类型运算*/
#define REG_DX 4
#define REG_EX 5/*用于变量赋值*/
#define REG_CF 6
#define REG_SP 7/*局部变量储存栈指针*/

/*Types*/
#define TYPE_INTEGER 0
#define TYPE_FLOAT 1

/*List Operations*/
#define LIST_UNIT_SIZE 10
#define LIST_DECLARE(type) \
        typedef struct{\
            int count;\
            int size;\
            type *vals;\
        }type##List;

#define LIST_INIT(list,type) \
        list.count=0;\
        list.size=LIST_UNIT_SIZE;\
        list.vals=(type*)malloc(LIST_UNIT_SIZE*sizeof(type));

#define LIST_DELETE(list) \
        free(list.vals);\
        list.vals=NULL;\
        list.count=0;\
        list.size=0;

#define LIST_ADD(list,type,val) \
        list.count++;\
        if(list.count>list.size){\
            list.size+=LIST_UNIT_SIZE;\
            list.vals=(type*)realloc(list.vals,sizeof(type)*list.size);\
        }\
        list.vals[list.count-1]=val;

#define LIST_SUB(list,type) \
    list.count--;\
    if(list.count<=(list.size-LIST_UNIT_SIZE)){\
        list.size-=LIST_UNIT_SIZE;\
        list.vals=(type*)realloc(list.vals,sizeof(type)*list.size);\
    }

#define LIST_CONNECT(list1,list2,type) \
    int licount=list1.count;\
    list1.count+=list2.count;\
    list1.vals=(type*)realloc(list1.vals,list1.count*sizeof(type));\
    list1.size=list1.count/LIST_UNIT_SIZE+1;\
    for(int li=licount;li<list1.count;li++){\
        list1.vals[li]=list2.vals[li-licount];\
    }
    
/*bool type*/
typedef enum{
    false=0,
    true
}bool;

typedef enum{
    TOKEN_END,
    TOKEN_UNKNOWN,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_WORD,
    TOKEN_INT,
    TOKEN_FLOAT_CLASS,
    TOKEN_FUNC,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_IF,
    TOKEN_ELIF,/*else if*/
    TOKEN_ELSE,
    TOKEN_EXCL,/*!*/
    TOKEN_CAND,
    TOKEN_COR,
    TOKEN_ADD,
    TOKEN_DOUBLE_ADD,/*++*/
    TOKEN_SUB,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_EQUAL,
    TOKEN_POINT,/*.*/
    TOKEN_COMMA,/*,*/
    TOKEN_SEMI,/*;*/
    TOKEN_PARE1,/*(*/
    TOKEN_PARE2,/*)*/
    TOKEN_BRACE1,/*{*/
    TOKEN_BRACE2,/*}*/
    TOKEN_BRACKET1,/*[*/
    TOKEN_BRACKET2,/*]*/
    TOKEN_GTHAN,/*>*/
    TOKEN_LTHAN,/*<*/
    TOKEN_INVERT,/*~*/
    TOKEN_NOT_EQUAL,/*!=*/
    TOKEN_GTHAN_EQUAL,/*>=*/
    TOKEN_LTHAN_EQUAL,/*<=*/
    TOKEN_AND,/*&*/
    TOKEN_OR,/*|*/
    TOKEN_LEFT,
    TOKEN_RIGHT,
    TOKEN_PUTC,
    TOKEN_ADD_EQUAL,
    TOKEN_SUB_EQUAL,
    TOKEN_MUL_EQUAL,
    TOKEN_DIV_EQUAL,
    TOKEN_AND_EQUAL,
    TOKEN_OR_EQUAL,
    TOKEN_LEFT_EQUAL,
    TOKEN_RIGHT_EQUAL
}TokenType;
typedef enum{
    DATA_REG,
    DATA_POINTER,/*指针*/
    DATA_INTEGER,
    DATA_REG_POINTER,/*指向寄存器里的指针*/
    DATA_STACK,
    DATA_REG_STACK
}DataType;
typedef enum{
    HANDLE_NOP,
    HANDLE_MOV,
    HANDLE_ADD,
    HANDLE_SUB,
    HANDLE_SUBS,/*负*/
    HANDLE_MUL,
    HANDLE_DIV,
    HANDLE_FADD,/*浮点数操作*/
    HANDLE_FSUB,
    HANDLE_FMUL,
    HANDLE_FDIV,
    HANDLE_EQUAL,
    HANDLE_JMP,/*跳转*/
    HANDLE_JMPC,/*条件跳转，当CF为0时跳转*/
    HANDLE_PUSH,/*栈*/
    HANDLE_POP,
    HANDLE_POPT,/*获得栈中指定的元素，0为栈顶,不会删除元素 popt [变量],[位置]*/
    HANDLE_GSP,
    //HANDLE_POPS,
    //HANDLE_PUSHS,
    HANDLE_PUSHB,/*压多个值*/
    HANDLE_SFREE,/*释放栈中指定数量的元素*/
    HANDLE_CAND,/*条件与*/
    HANDLE_COR,/*条件或*/
    HANDLE_CALL,
    HANDLE_RET,
    HANDLE_AND,
    HANDLE_OR,
    HANDLE_INVERT,/*非*/
    HANDLE_INVERT2,/*桉位取反*/
    HANDLE_GTHAN,
    HANDLE_GTHAN_EQUAL,
    HANDLE_LTHAN,
    HANDLE_LTHAN_EQUAL,
    HANDLE_NOT_EQUAL,
    HANDLE_LEFT,
    HANDLE_RIGHT,
    HANDLE_FTOI,/*浮点小数转化为整数 */
    HANDLE_PUTC/*输出一个字符*/
}HandleType;
LIST_DECLARE(int)
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
LIST_DECLARE(Cmd)
typedef struct{
    char name[WORD_MAX];
    int size;/*整数的数量*/
}ClassType;
LIST_DECLARE(ClassType)
typedef struct Varb{
    char name[WORD_MAX];
    int class;
    int ptr;
    int dim;/*维度,0为普通变量*/
    int unitSize;
    struct Varb*subVar;
    int arrayCount;
    enum{
       VAR_GLOBAL,
       VAR_PART,
       VAR_PARAC
    }vtype;
    //bool isArray;
    //intList arrayCount;/*第一项为数组的总大小,最后一项为数组的最小单位大小 */
}Variable;
LIST_DECLARE(Variable)
typedef struct{
    VariableList*pvlist;
    intList*breakList;
}Environment;
typedef struct{
    char*fileName;
    char*code;
    int ptr;
    int line;
    int dataSize;
    VariableList varlist;
    ClassTypeList classList;
    CmdList exeClist;/*用于直接执行的指令*/
}Parser;
typedef struct{
    int class;
    bool isVar;
    int dim;
    bool isStack;
}ReturnType;
void clistToString(char*text,CmdList clist,bool isNum);
void vlistToString(char*text,VariableList vlist);
void initParser(Parser*parser);
void reportError(Parser*parser,char*msg);
void reportWarning(Parser*parser,char*msg);
void connectCmdList(CmdList*clist,CmdList newClist);
void addCmd1(CmdList*clist,HandleType ht,DataType ta,int a);
void addCmd2(CmdList*clist,HandleType ht,DataType ta,DataType tb,int a,int b);
#endif