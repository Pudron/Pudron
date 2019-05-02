#include"normal.h"
#include"pio.h"
#include"parse.h"
int main(int argc,char**argv){
    Parser parser;
    if(argc==1){
        puts("Welcome to use Pudron Program Language\nusage:pdc [file name]\n");
        return 0;
    }
    initParser(&parser);
    parser.fileName=argv[1];
    if(!readTextFile(&parser.code,argv[1])){
        return -1;
    }
    Environment envirn;
    CmdList clist;
    char text[1024];
    LIST_INIT(clist,Cmd);
    if(getExpression(&parser,&clist,envirn)){
        clistToString(text,clist);
        printf("CmdList:\n%s\n",text);
    }else{
        printf("No expression\n");
    }
    LIST_DELETE(clist);
    return 0;
}