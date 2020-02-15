#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"pdex.h"
PD_FUNC_DEF(readTextFile){
    if(PD_ARG_COUNT!=1){
        PD_ERROR(PERROR_ARGUMENT,"only expected file name argument.");
    }
    _PValue vstr=PD_ARG(0);
    if(vstr.type!=PVAL_STRING){
        PD_ERROR(PERROR_ARGUMENT,"expected file name argument.");
    }
    FILE*fp=fopen(vstr.str,"rt");
    if(fp==NULL){
        PD_ERROR(PERROR_FILE,"can not open the file.");
    }
    fseek(fp,0,SEEK_END);
    int len=ftell(fp);
    rewind(fp);
    char*text=(char*)malloc(len+1);
    if(text==NULL){
        PD_ERROR(PERROR_MEMORY,"memory error.");
    }
    len=fread(text,1,len,fp);
    fclose(fp);
    text[len]='\0';
    PD_RETURN_STRING(text);
}
PD_FUNC_DEF(writeTextFile){
    if(PD_ARG_COUNT!=2){
        PD_ERROR(PERROR_ARGUMENT,"only expected 2 argument.");
    }
    _PValue vfname=PD_ARG(0);
    if(vfname.type!=PVAL_STRING){
        PD_ERROR(PERROR_ARGUMENT,"expected file name argument.");
    }
    _PValue vstr=PD_ARG(1);
    if(vstr.type!=PVAL_STRING){
        PD_ERROR(PERROR_ARGUMENT,"expected output text argument.");
    }
    FILE*fp=fopen(vfname.str,"wt");
    if(fp==NULL){
        PD_ERROR(PERROR_FILE,"can not open the file.");
    }
    fwrite(vstr.str,1,strlen(vstr.str),fp);
    fclose(fp);
}