#ifndef _PD_COMMON_H_
#define _PD_COMMON_H_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdarg.h>
/*尽量不用memcpy(),用它老是出现莫名其妙的错误,用它操作过的指针free()时都出错*/
#define MAX_WORD_LENGTH 128
#define FILE_CODE_POSTFIX ".pd"
#define FILE_MODULE_POSTFIX ".pdm"
#define FILE_SIGN 5201314
#define VERSION 1
#define VERSION_MIN 1

#define STD_CLASS_COUNT 6
#define STD_FUNC_COUNT 4

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
        if(list.vals!=NULL){\
            free(list.vals);\
        }\
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
        list.vals=(type*)memManage(list.vals,sizeof(type)*list.size);\
    }

#define LIST_CONNECT(list1,list2,type) \
	list1.count+=list2.count;\
    if(list1.count>list1.size){\
        list1.size=pow2(list1.count);\
        list1.vals=(type*)memManage(list1.vals,list1.size*sizeof(type));\
    }\
    memcpy(list1.vals+list2.count,list2.vals,list2.count*sizeof(type));

#define LIST_INSERT(list,type,position,val) /*在position前插入值,[position]的值即为val*/\
    list.count++;\
    if(list.count>list.size){\
        list.size=pow2(list.count);\
        list.vals=(type*)memManage(list.vals,sizeof(type)*list.size);\
    }\
    memcpy(list.vals+position+1,list.vals+position,(list.count-position)*sizeof(type));\
    list.vals[position]=val;

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
    TOKEN_FUNC,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_FOR,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
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
    TOKEN_FALSE
}Tokentype;
#define OPCODE_COUNT 40
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
    OPCODE_LOAD_MEMBER,
    OPCODE_LOAD_SUBSCRIPT,
    OPCODE_STACK_COPY,
    OPCODE_POP_STACK,/*参数为数量*/
    OPCODE_JUMP,
    OPCODE_JUMP_IF_FALSE,

    OPCODE_CALL_FUNCTION,
    OPCODE_RETURN,
    OPCODE_INVERT_ORDER,/*arg:count,倒序*/
    OPCODE_SET_ASSIGN_COUNT,/*set asc*/
    OPCODE_ASSIGN,/*arg:operation type(-1 is normal assignment),顺便把asc设为1*/

    OPCODE_MAKE_ARRAY,
    OPCODE_GET_FOR_INDEX,
    
    OPCODE_LOAD_METHOD,/*不POP前面的对象*/
    OPCODE_CALL_METHOD,/*创建对象环境*/

    OPCODE_CLASS_EXTEND,/*继承*/
    OPCODE_LOAD_MODULE
}Opcode;
LIST_DECLARE(int)
LIST_DECLARE(char)
typedef unsigned int uint;
LIST_DECLARE(uint)
typedef char* Name;
LIST_DECLARE(Name);
typedef struct{
    Tokentype type;
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
typedef struct ObjectDef Object;
typedef struct{
    char*name;
    int nextSlot;
    bool isUsed;
    Object*obj;
}HashSlot;
typedef struct{
    int capacity;
    HashSlot*slot;
}HashList;
struct ModuleDef{
    char*name;
    /*unit start*/
    ConstList constList;
    intList clist;
    ModuleList moduleList;
    PartList partList;
    HashList lvlist;
    NameList nlist;
    /*unit end*/
};
typedef struct VMDef VM;
typedef struct UnitDef Unit;
typedef struct{
    char*name;
    void (*exe)(VM*,Unit*);
    int argCount;/*nlist的前argCount个名字皆为参数*/
    /*unit start*/
    ConstList constList;
    intList clist;
    ModuleList moduleList;
    PartList partList;
    HashList lvlist;
    NameList nlist;
    /*unit end*/
}Func;
typedef struct ClassDef Class;
LIST_DECLARE(Class)
struct ClassDef{
    /*确定对象类型时用字符串来确定,创建标准对象只能通过常量方式*/
    char*name;
    ClassList parentList;
    HashList memberList;
    NameList varList;/*成员名字,调用initFunc时有用,不包括父类名*/
    Func initFunc;/*用于初始化成员，执行完后手动将栈中的结果赋予成员*/
};
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
        Class class;
    };
};
struct ObjectDef{
    enum{
        OBJECT_INT,
        OBJECT_DOUBLE,
        OBJECT_CLASS,
        OBJECT_FUNCTION,
        OBJECT_STRING,
        OBJECT_LIST,
        OBJECT_OTHERS
    }type;
    NameList classNameList;
    union{
        int num;
        double numd;
        char*str;
        Func func;
        Class class;
        Object**subObj;/*用于list*/
    };
    HashList member;
    int refCount;
    bool isInit;/*用于标记初始化对象*/
};
struct UnitDef{
    ConstList constList;
    intList clist;
    ModuleList mlist;/*第一个module为当前空module,只用其名字来防止重复引用*/
    PartList plist;
    NameList nlist;/*namespace*/
    HashList gvlist;/*global variables(include upvalues)*/
    HashList lvlist;/*local variables*/
    int curPart;
    int ptr;
};
typedef struct{
    Opcode opcode;
    char*name;
    int argCount;
}OpcodeMsg;
typedef struct{
    HashList hl;
    Class stdClass[STD_CLASS_COUNT];
    Func stdFunc[STD_FUNC_COUNT];
}PdSTD;
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
Unit newUnit();
void setModuleUnit(Module*mod,Unit unit);
void setFuncUnit(Func*func,Unit unit);
Unit getModuleUnit(Module mod);
Unit getFuncUnit(Func func);
void printCmds(Unit unit,int blankCount);
int addName(NameList*nlist,char*name);
/*尽量在执行之前就建立好hash表，扩容时开销挺大
*未找到则返回-1,若obj未NULL,则不给找到的slot赋值*/
int hashGet(HashList*hl,char*name,Object*obj,bool isAdd);
HashList newHashList();
void expandHashList(HashList*hl,int size);
HashList hashMerge(HashList hl1,HashList hl2);
HashList hashCopy(HashList hl);
void hashPrint(HashList hl);
#endif