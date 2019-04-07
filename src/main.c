#include"normal.h"
#include"pio.h"
#include"parse.h"
int printCmds(Commands cmds){
    puts("numlist:\n");
    for(int i=0;i<cmds.count;i++){
        printf("%d ",cmds.vals[i]);
        if((i%8)==0 && i!=0){
            putchar('\n');
        }
    }
    puts("\nnumlist end\n");
    return 0;
}
int main(int argc,char**argv){
    Msg msg;
    Parser parser;
    if(argc==1){
        puts("Welcome to use Pudron Program Language\nusage:pdc [file name]\n");
        return 0;
    }
    initParser(&parser);
    parser.fileName=argv[1];
    msg=readTextFile(&parser.code,argv[1]);
    if(msg.type==MSG_ERROR){
        printf("error:%s\n",msg.text);
        return 1;
    }
    msg=parse(&parser);
    if(msg.type!=MSG_SUCCESS){
        printf("%s\n",msg.text);
        return 1;
    }
    char txt[500];
    Commands cmds;
    cmds.vals=(unsigned char*)malloc(parser.clist.memory);
    clistToCmds(&cmds,parser.clist);
    cmdToString(txt,cmds);
    printf("Cmds:\n%s\n",txt);
    return 0;
}