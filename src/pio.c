#include"pio.h"
bool readTextFile(char**text,char*fileName){
    FILE*fp=fopen(fileName,"r");
    if(fp==NULL){
        printf("error:an not open the file \"%s\"!",fileName);
        return false;
    }
    fseek(fp,0,SEEK_END);
    int len=ftell(fp);
    rewind(fp);
    *text=(char*)malloc(len+1);
    len=fread(*text,1,len,fp);
    fclose(fp);
    (*text)[len]='\0';
    return true;
}