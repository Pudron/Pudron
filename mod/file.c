#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"pdex.h"
PD_FUNC_DEF(readTextFile){
    if(PD_ARG_COUNT!=1){
        PD_ERROR(PERROR_ARGUMENT,L"only expected file name argument.");
    }
    _PValue vstr=PD_ARG(0);
    if(vstr.type!=PVAL_STRING){
        PD_ERROR(PERROR_ARGUMENT,L"expected file name argument.");
    }
    size_t leng=wcslen(vstr.str);
    char*fname=(char*)malloc(leng+1);
    leng=wcstombs(fname,vstr.str,leng);
    fname[leng]='\0';
    FILE*fp=fopen(fname,"rt");
    free(fname);
    if(fp==NULL){
        PD_ERROR(PERROR_FILE,L"can not open the file.");
    }
    fseek(fp,0,SEEK_END);
    int len=ftell(fp);
    rewind(fp);
    wchar_t*text=(wchar_t*)malloc((len+1)*sizeof(wchar_t));
    if(text==NULL){
        PD_ERROR(PERROR_MEMORY,L"memory error.");
    }
    len=fread(text,sizeof(wchar_t),len,fp);
    fclose(fp);
    text[len]=L'\0';
    PD_RETURN_STRING(text);
}
PD_FUNC_DEF(writeTextFile){
    if(PD_ARG_COUNT!=2){
        PD_ERROR(PERROR_ARGUMENT,L"only expected 2 argument.");
    }
    _PValue vfname=PD_ARG(0);
    if(vfname.type!=PVAL_STRING){
        PD_ERROR(PERROR_ARGUMENT,L"expected file name argument.");
    }
    _PValue vstr=PD_ARG(1);
    if(vstr.type!=PVAL_STRING){
        PD_ERROR(PERROR_ARGUMENT,L"expected output text argument.");
    }
    size_t leng=wcslen(vfname.str);
    char*fname=(char*)malloc(leng+1);
    wcstombs(fname,vfname.str,leng);
    fname[leng]='\0';
    FILE*fp=fopen(fname,"wt");
    free(fname);
    if(fp==NULL){
        PD_ERROR(PERROR_FILE,L"can not open the file.");
    }
    fwrite(vstr.str,sizeof(wchar_t),wcslen(vstr.str),fp);
    fclose(fp);
}