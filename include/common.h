#ifndef _PD_COMMON_H_
#define _PD_COMMON_H_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_WORD_LENGTH 20
#define FILE_POSTFIX ".pd"
#define FILE_LIB_POSTFIX ".pdl"
#define FILE_SIGN 5201314
#define VERSION 1
#define VERSION_MIN 1

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
#define LIST_REDUCE(list,type,mcount) \
    list.count-=mcount;\
    if(list.count<=(list.size-LIST_UNIT_SIZE)){\
        list.size-=mcount;\
        list.vals=(type*)realloc(list.vals,sizeof(type)*list.size);\
    }

#define LIST_CONNECT(list1,list2,type,id) \
    int licount##id=list1.count;\
    list1.count+=list2.count;\
    list1.vals=(type*)realloc(list1.vals,list1.count*sizeof(type));\
    list1.size=list1.count;\
    for(int li=licount##id;li<list1.count;li++){\
        list1.vals[li]=list2.vals[li-licount##id];\
    }

/*bool type*/
typedef enum{
    false=0,
    true
}bool;

#define STD_CLASS_COUNT 6
#define CLASS_INT 0
#define CLASS_CLASS 1
#define CLASS_FUNCTION 2
#define CLASS_META 3
#define CLASS_FLOAT 4
#define CLASS_STRING 5

typedef enum{
    TOKEN_END,
    TOKEN_UNKNOWN,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_WORD,
    TOKEN_STRING,
    TOKEN_FUNC,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_BREAK,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELIF,/*else if*/
    TOKEN_ELSE,
    TOKEN_EXCL,/*!*/
    TOKEN_CAND,
    TOKEN_COR,
    TOKEN_ADD,
    //TOKEN_DOUBLE_ADD,/*++*/
    //TOKEN_DOUBLE_SUB,/*--*/
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
    TOKEN_PERCENT,/*%*/
    TOKEN_COLON,/*:*/
    TOKEN_LEFT,
    TOKEN_RIGHT,
    TOKEN_ADD_EQUAL,
    TOKEN_SUB_EQUAL,
    TOKEN_MUL_EQUAL,
    TOKEN_DIV_EQUAL,
    TOKEN_AND_EQUAL,
    TOKEN_PERCENT_EQUAL,/*%=*/
    TOKEN_OR_EQUAL,
    TOKEN_LEFT_EQUAL,
    TOKEN_RIGHT_EQUAL,
    TOKEN_CLASS,
    TOKEN_IMPORT,
    TOKEN_INCLUDE,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_LINE
}TokenType;
#define OPT_METHOD_COUNT 18
#define OPCODE_COUNT 58
typedef enum{
    OPCODE_NOP,
    OPCODE_ADD,
    OPCODE_SUB,
    OPCODE_MUL,
    OPCODE_DIV,
    OPCODE_AND,
    OPCODE_OR,
    OPCODE_CAND,
    OPCODE_COR,
    OPCODE_LEFT,
    OPCODE_RIGHT,
    OPCODE_EQUAL,
    OPCODE_GTHAN,
    OPCODE_LTHAN,
    OPCODE_NOT_EQUAL,
    OPCODE_GTHAN_EQUAL,
    OPCODE_LTHAN_EQUAL,
    OPCODE_REM,

    OPCODE_INVERT,
    OPCODE_NOT,
    OPCODE_SUBS,
    
    OPCODE_LOAD_CONST,
    OPCODE_LOAD_VAL,
    OPCODE_LOAD_ATTR,
    OPCODE_LOAD_INDEX,
    OPCODE_STORE_VAL,
    OPCODE_STORE_ATTR,
    OPCODE_STORE_INDEX,
    OPCODE_PUSH_VAL,
    OPCODE_STACK_COPY,
    OPCODE_POP_VAR,/*参数为数量*/
    OPCODE_POP_STACK,/*参数为数量*/
    OPCODE_JUMP,
    OPCODE_JUMP_IF_FALSE,
    OPCODE_SET_FIELD,
    OPCODE_FREE_FIELD,
    OPCODE_SET_LOOP,
    OPCODE_FREE_LOOP,

    OPCODE_CALL_FUNCTION,
    OPCODE_CALL_METHOD,
    OPCODE_RETURN,
    OPCODE_ENABLE_FUNCTION,/*顺便把函数变量入栈*/

    OPCODE_MAKE_OBJECT,
    OPCODE_EXTEND_CLASS,
    OPCODE_ENABLE_CLASS,

    OPCODE_SET_MODULE,/*参数为模块名*/
    OPCODE_RETURN_MODULE,

    OPCODE_PRINT_STACK,
    OPCODE_PRINT_VAR,
    OPCODE_PRINT_FUNC,
    OPCODE_PRINT_CLASS,

    OPCODE_GET_VARCOUNT,
    OPCODE_RESIZE_VAR,
    OPCODE_MAKE_ARRAY,
    OPCODE_GET_CLASS,
    OPCODE_EXIT,

    OPCODE_MAKE_RANGE
}Opcode;
LIST_DECLARE(int)
LIST_DECLARE(char)
typedef char* Name;
LIST_DECLARE(Name);
typedef struct{
    TokenType type;
    union{
        int num;
        float numf;
        char*word;
    };
}Token;
typedef struct{
    enum{
        MSG_ERROR,
        MSG_WARNING
    }type;
    char*code;
    char*fileName;
    char text[100];
    int line,column,start,end;
}Msg;
typedef struct{
    char*code,*fileName;
    int line,column,start,end;
}Part;
LIST_DECLARE(Part)
typedef struct{
    int class;
    union{
        int num;
        float numf;
    };
    int refID;
}Value;
LIST_DECLARE(Value)
typedef struct{
    enum{
        SYM_INT,
        SYM_FLOAT,
        SYM_STRING,
    }type;
    union{
        int num;
        float numf;
        char*str;
    };
}Symbol;
LIST_DECLARE(Symbol)
typedef struct{
    int code[10];
    int count;
}Command;
typedef struct{
    char*name;
    int funcBase;
    int cmdBase;
    int symBase;
    int classBase;
    int partBase;
}Module;
LIST_DECLARE(Module)
typedef struct{
    char*name;
    NameList args;
    intList clist;
    int moduleID;
}Func;
LIST_DECLARE(Func)
typedef struct{
    char*name;
    NameList var;
    int varBase;
    FuncList methods;
    Func optMethod[OPT_METHOD_COUNT];
    intList parentList;
    int initID,destroyID;
}Class;
LIST_DECLARE(Class)
typedef struct{
    char*fileName;
    char*code;
    int ptr;
    int line,column;
    int curPart;
    bool isLib;
    int curModule;
    ModuleList moduleList;
    PartList partList;
    intList clist;
    SymbolList symList;
    FuncList funcList;
    ClassList classList;
}Parser;
typedef struct{
    Opcode opcode;
    char*name;
    bool isArg;
    bool isSymbol;
}OpcodeMsg;
void initParser(Parser*parser,bool isRoot);
void freeParser(Parser*parser);
//void extend(Class*class,Class eclass);
char*cutText(char*text,int start,int end);
char*cutPostfix(char*text);
char*getPostfix(char*text);
char*cutPath(char*text);
void reportMsg(Msg msg);
void reportError(Parser*parser,char*text,int start);
void reportWarning(Parser*parser,char*text,int start);
void addCmd(Parser*parser,intList*clist,int opcode);
void addCmd1(Parser*parser,intList*clist,int opcode,int dat);
void addCmds(Parser*parser,intList*clist,Command cmds);
int addSymbol(Parser*parser,Symbol symbol);
void clistToString(Parser parser,intList clist,char*text,Module module);
void funcToString(Parser parser,FuncList funcList,char*text);
void classToString(Parser parser,char*text);
#endif