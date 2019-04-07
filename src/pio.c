#include"pio.h"
Msg readTextFile(char**text,char*fileName){
    Msg msg;
    FILE*fp=fopen(fileName,"r");
    if(fp==NULL){
        msg.type=MSG_ERROR;
        sprintf(msg.text,"Can not open the file \"%s\"!",fileName);
        return msg;
    }
    fseek(fp,0,SEEK_END);
    int len=ftell(fp);
    rewind(fp);
    *text=(char*)malloc(len+1);
    len=fread(*text,1,len,fp);
    fclose(fp);
    (*text)[len]='\0';
    msg.type=MSG_SUCCESS;
    return msg;
}