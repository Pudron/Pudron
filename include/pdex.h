#ifndef _PD_EXTEND_H_
#define _PD_EXTEND_H_
/*此文件为Pudron动态链接库扩展提供模板*/
typedef struct{
    enum{
        PVAL_INT,
        PVAL_DOUBLE,
        PVAL_STRING
    }type;
    union{
        int num;
        double numd;
        char*str;
    };
}_PValue;/*默认为整数0*/
typedef struct{
    int count;
    int size;
    _PValue*vals;
}_PValueList;
typedef struct{
    _PValue rval;
    _PValueList argList;
}_PDat;
#define PD_FUNCTION_DEF(func_name) void func_name(_PDat*_pdat)
#define PD_ARG(index) _pdat->argList.vals[index]
#define PD_ARG_COUNT _pad->argList.count
#define PD_RETURN_INT(val) _pdat->rval.type=PVAL_INT;_pdat->rval.num=val;return
#define PD_RETURN_DOUBLE(val) _pdat->rval.type=PVAL_DOUBLE;_pdat->rval.numd=val;return
#define PD_RETURN_STRING(val) _pdat->rval.type=PVAL_STRING;_pdat->rval.str=val;return
#endif