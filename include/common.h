#ifndef _PD_COMMON_H_
#define _PD_COMMON_H_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_WORD_LENGTH 20

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
    list1.size=list1.count;\
    for(int li=licount;li<list1.count;li++){\
        list1.vals[li]=list2.vals[li-licount];\
    }

/*bool type*/
typedef enum{
    false=0,
    true
}bool;
enum{
    CLASS_INDEX,
    CLASS_FUNCTION,
    CLASS_INT,
    CLASS_FLOAT,
    CLASS_STRING
};
typedef enum{
    TOKEN_END,
    TOKEN_UNKNOWN,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_WORD,
    TOKEN_FUNC,
    TOKEN_WHILE,
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
    TOKEN_PUTC,
    TOKEN_ADD_EQUAL,
    TOKEN_SUB_EQUAL,
    TOKEN_MUL_EQUAL,
    TOKEN_DIV_EQUAL,
    TOKEN_AND_EQUAL,
    TOKEN_PERCENT_EQUAL,/*%=*/
    TOKEN_OR_EQUAL,
    TOKEN_LEFT_EQUAL,
    TOKEN_RIGHT_EQUAL,
    TOKEN_CLASS
}TokenType;
typedef enum{
    OPCODE_NOP,
    OPCODE_ADD,
    OPCODE_SUB,
    OPCODE_MUL,
    OPCODE_DIV,
    OPCODE_AND,
    OPCODE_OR,
    OPCODE_LEFT,
    OPCODE_RIGHT,
    OPCODE_EQUAL,
    OPCODE_GTHAN,
    OPCODE_LTHAN,
    OPCODE_NOT_EQUAL,
    OPCODE_GTHAN_EQUAL,
    OPCODE_LTHAN_EQUAL,
    OPCODE_INVERT,
    OPCODE_NOT,
    OPCODE_REM,
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
    OPCODE_SET_WHILE,
    OPCODE_FREE_WHILE,

    OPCODE_CALL_FUNCTION,
    OPCODE_CALL_METHOD,
    OPCODE_RETURN,
    OPCODE_ENABLE_FUNCTION,

    OPCODE_MAKE_OBJECT,
    OPCODE_EXTEND_CLASS,

    OPCODE_LOAD_CLASS,
    OPCODE_LOAD_FUNCTION
}Opcode;
LIST_DECLARE(int)
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
    char text[100];
    int line,column,start,end;
}Msg;
typedef struct{
    int line,column,start,end;
}Part;
LIST_DECLARE(Part)
typedef struct ValueDef{
    int class;
    union{
        int num;
        float numf;
        char*str;
    };
    struct ValueDef*var;
    union{
        int refID;
        int refCount;
    };
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
    NameList args;
    intList clist;
}Func;
LIST_DECLARE(Func)
typedef struct{
    char*name;
    NameList var;
    FuncList methods;
    Func addMethod,subMethod,mulMethod,divMethod,leftMethod,rightMethod,equalMethod;
}Class;
LIST_DECLARE(Class)
typedef struct{
    char*fileName;
    char*code;
    int ptr;
    int line,column;
    int curPart;
    PartList partList;
    intList clist;
    SymbolList symList;
    FuncList funcList;
    ClassList classList;
}Parser;
void initParser(Parser*parser);
void reportMsg(Parser*parser,Msg msg);
void reportError(Parser*parser,char*text,int start);
void reportWarning(Parser*parser,char*text,int start);
void addCmd(Parser*parser,intList*clist,int opcode);
void addCmd1(Parser*parser,intList*clist,int opcode,int dat);
void addCmds(Parser*parser,intList*clist,Command cmds);
int addSymbol(Parser*parser,Symbol symbol);
void clistToString(Parser parser,intList clist,char*text);
void funcToString(Parser parser,FuncList funcList,char*text);
void classToString(Parser parser,char*text);
#endif