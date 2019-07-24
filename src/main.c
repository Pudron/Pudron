#include"normal.h"
#include"pio.h"
#include"compiler.h"
#include"exec.h"
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
    compile(&parser);
    char text[500];
    clistToString(text,parser.exeClist);
    printf("Clist:\n%s\n",text);
    vlistToString(text,parser.varlist);
    printf("varlist:\n%s\n",text);
    VM vm;
    initVM(&vm,parser);
    execute(&vm);
    dataToString(text,vm);
    printf("data:\n%s\n",text);
    return 0;
}