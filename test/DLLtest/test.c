#include<stdio.h>
#include"../../include/pdex.h"
PD_FUNC_DEF(test){
    if(PD_ARG(0).type==PVAL_INT){
        wprintf(L"arg:%d\n",PD_ARG(0).num);
    }else if(PD_ARG(0).type==PVAL_STRING){
        wprintf(L"get:%s\n",PD_ARG(0).str);
    }else{
        wprintf(L"Hello\n");
    }
    PD_RETURN_INT(12345);
}