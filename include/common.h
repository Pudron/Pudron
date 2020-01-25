#ifndef _PD_COMMON_H_
#define _PD_COMMON_H_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_WORD_LENGTH 128
#define FILE_POSTFIX ".pd"
#define FILE_LIB_POSTFIX ".pdm"
#define FILE_SIGN 5201314
#define VERSION 1
#define VERSION_MIN 1

/*List Operations*/
//#define LIST_UNIT_SIZE 10
#define LIST_DECLARE(type) \
        typedef struct{\
            int count;\
            int size;\
            type *vals;\
        }type##List;

#define LIST_INIT(list) \
        list.count=0;\
        list.size=0;\
        list.vals=NULL;

#define LIST_DELETE(list) \
        free(list.vals);\
        list.vals=NULL;\
        list.count=0;\
        list.size=0;

#define LIST_ADD(list,type,val) \
        list.count++;\
        if(list.count>list.size){\
            list.size=pow2(list.count);\
            list.vals=(type*)memManage(list.vals,sizeof(type)*list.size);\
        }\
        list.vals[list.count-1]=val;

#define LIST_SUB(list,type) \
    list.count--;\
    list.size=pow2(list.count);\
    if(list.count<=list.size){\
        list.vals=(type*)memManage(list.vals,sizeof(type)*list.size);\
    }
#define LIST_REDUCE(list,type,mcount) \
    list.count-=mcount;\
    list.size=pow2(list.count);\
    if(list.count<=list.size){\
        list.vals=(type*)realloc(list.vals,sizeof(type)*list.size);\
    }

#define LIST_CONNECT(list1,list2,type) \
	list1.count+=list2.count;\
    if(list1.count>list1.size){\
        list1.size=pow2(list1.count);\
        list1.vals=(type*)memManage(list1.vals,list1.size*sizeof(type));\
    }\
    memcpy(list1.vals+list2.count,list2.vals,list2.count*sizeof(type));

/*bool type*/
typedef enum{
    false=0,
    true
}bool;

typedef enum{
    TOKEN_END,
    TOKEN_UNKNOWN,
    TOKEN_INTEGER,
    TOKEN_DOUBLE,
    TOKEN_WORD,
    TOKEN_STRING,
    TOKEN_FUNCTION,
    TOKEN_WHILE,
    TOKEN_DO,
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
    TOKEN_ARG
}TokenType;
#define OPT_METHOD_COUNT 17
#define OPCODE_COUNT 45
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
    OPCODE_LOAD_VAR,
    OPCODE_LOAD_ATTR,/*类内部访问*/
    OPCODE_LOAD_MEMBER,
    OPCODE_LOAD_SUBSCRIPT,
    //OPCODE_PUSH_VAL,
    OPCODE_STACK_COPY,
    //OPCODE_POP_VAR,/*参数为数量*/
    //OPCODE_PUSH_STACK,/*arg:count*/
    OPCODE_POP_STACK,/*参数为数量*/
    OPCODE_JUMP,
    OPCODE_JUMP_IF_FALSE,
    OPCODE_LOAD_FIELD,
    OPCODE_FREE_FIELD,
    OPCODE_SET_LOOP,
    OPCODE_FREE_LOOP,

    OPCODE_CALL_FUNCTION,
    //OPCODE_CALL_METHOD,/*arg:index,count*/
    //OPCODE_CALL_ATTR,/*类内部访问*/
    OPCODE_RETURN,
    //OPCODE_ENABLE_FUNCTION,/*顺便把函数变量入栈*/
    OPCODE_INVERT_ORDER,/*arg:count,倒序*/
    //OPCODE_ASSIGN_LEFT,/*arg:count*/
    //OPCODE_ASSIGN_RIGHT,/*arg:count*/
    OPCODE_SET_ASSIGN_COUNT,/*set asc*/
    OPCODE_ASSIGN,/*arg:operation type(-1 is normal assignment),顺便把asc设为1*/

    OPCODE_MAKE_ARRAY,
    OPCODE_GET_FOR_INDEX,
    OPCODE_LOAD_STACK,/*以当前unit的start为基准*/
    OPCODE_LOAD_ARG_COUNT
    /*OPCODE_MAKE_OBJECT,
    OPCODE_EXTEND_CLASS,
    OPCODE_ENABLE_CLASS,

    OPCODE_SET_MODULE,
    OPCODE_RETURN_MODULE,

    OPCODE_PRINT_STACK,
    OPCODE_PRINT_VAR,
    OPCODE_PRINT_FUNC,
    OPCODE_PRINT_CLASS,

    OPCODE_GET_VARCOUNT,
    OPCODE_RESIZE_VAR,
    OPCODE_GET_CLASS,
    OPCODE_GET_VARBASIS,
    OPCODE_SET_VARBASIS,
    OPCODE_EXIT,

    OPCODE_MAKE_RANGE,
    OPCODE_COPY_OBJECT,
    OPCODE_STR_FORMAT,
    OPCODE_PRINT,
    OPCODE_INPUT,
    OPCODE_STR_COMPARE,
    OPCODE_READ_TEXT_FILE,
    OPCODE_WRITE_TEXT_FILE,

    OPCODE_DLL_OPEN,
    OPCODE_DLL_CLOSE,
    OPCODE_DLL_EXECUTE*/
}Opcode;
LIST_DECLARE(int)
LIST_DECLARE(char)
typedef unsigned int uint;
LIST_DECLARE(uint)
typedef char* Name;
LIST_DECLARE(Name);
typedef struct{
    TokenType type;
    union{
        int num;
        double numd;
        char*word;
    };
    int start,end,column,line;
}Token;
LIST_DECLARE(Token)
typedef struct{
    enum{
        MSG_ERROR,
        MSG_WARNING
    }type;
    char*code;
    char*fileName;
    char text[500];
    int line,column,start,end;
}Msg;
typedef struct{
    char*code,*fileName;
    int line,column,start,end;
}Part;
LIST_DECLARE(Part)
typedef struct{
    bool isRef;
    char*name;
    int hashName;
}Var;
LIST_DECLARE(Var)
typedef struct{
    VarList varList;
}Field;
LIST_DECLARE(Field)
typedef struct{
    int code[10];
    int count;
}Command;
typedef struct{
    char*fileName;
    char*code;
    int curToken,ptr,line,column;
    TokenList tokenList;
}Parser;
typedef struct ModuleDef Module;
typedef struct ConstDef Const;
LIST_DECLARE(Const)
LIST_DECLARE(Module)
typedef struct{
    char*name;
    int hashName;
}Member;
LIST_DECLARE(Member)
struct ModuleDef{
    char*name;
    /*unit start*/
    ConstList constList;
    intList clist;
    ModuleList moduleList;
    PartList partList;
    FieldList fieldList;
    MemberList memberList;
    /*unit end*/
};
typedef struct VMDef VM;
typedef struct UnitDef Unit;
typedef struct{
    void (*exe)(VM*,Unit*);
    VarList argList;
    /*unit start*/
    ConstList constList;
    intList clist;
    ModuleList moduleList;
    PartList partList;
    FieldList fieldList;
    MemberList memberList;
    /*unit end*/
}Func;
typedef struct{
    char*name;
    uint hashName;
    VarList varList;
    Func initFunc;/*用于初始化成员，执行完后手动将栈中的结果赋予成员*/
    int optID[OPT_METHOD_COUNT];
    int initID,destroyID,subID;
}Class;
struct ConstDef{
    enum{
        CONST_INT,
        CONST_DOUBLE,
        CONST_STRING,
        CONST_FUNCTION,
        CONST_CLASS
    }type;
    union{
        int num;
        double numd;
        char*str;
        Func func;
        Class classd;
    };
};
typedef struct ObjectDef{
    Class*class;
    union{
        int num;
        double numd;
        char*str;
        Func func;
        Class*classd;
    };
    struct ObjectDef**objs;
    int varCount;
    int refCount;
}Object;
struct UnitDef{
    ConstList constList;
    intList clist;
    ModuleList mlist;
    PartList plist;
    FieldList flist;
    MemberList mblist;
    int varStart;
    int curPart;
    int ptr;
    int argc;
};
typedef struct{
    Opcode opcode;
    char*name;
    int argCount;
}OpcodeMsg;

void*memManage(void*ptr,size_t size);
char*cutText(char*text,int start,int end);
char*cutPostfix(char*text);
char*getPostfix(char*text);
char*cutPath(char*text);
char*getPath(char*text);
/*pow2():求大于等于num的最小２次冪*/
int pow2(int num);
void reportMsg(Msg msg);
unsigned int hashString(char*str);
Unit newUnit(int varStart);
void setModuleUnit(Module*mod,Unit unit);
void setFuncUnit(Func*func,Unit unit);
Unit getModuleUnit(Module mod);
Unit getFuncUnit(Func func);
void printCmds(Unit unit);
#endif