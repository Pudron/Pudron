#include"normal.h"
#include"pio.h"
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
    //compile(&parser);
    return 0;
}