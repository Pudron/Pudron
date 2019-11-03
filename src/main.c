#include"common.h"
#include"pio.h"
#include"parser.h"
void testToken(Parser*parser){
    Token token;
    token=nextToken(parser);
    while(token.type!=TOKEN_END){
        if(token.type==TOKEN_WORD){
            printf("%d:%s\n",token.type,token.word);
        }else if(token.type==TOKEN_INTEGER){
            printf("%d:%d\n",token.type,token.num);
        }else if(token.type==TOKEN_FLOAT){
            printf("%d:%f\n",token.type,token.numf);
        }else{
            printf("%d\n",token.type);
        }
        token=nextToken(parser);
    }
}
void testCompile(Parser*parser){
    Env env={-1,false,NULL,true,false};
    getBlock(parser,&parser->clist,env);
    char text[1000];
    classToString(*parser,text);
    printf("%s",text);
    funcToString(*parser,parser->funcList,text);
    printf("%s",text);
    clistToString(*parser,parser->clist,text);
    printf("clist(size:%d):\n%s\n",parser->clist.count,text);
}
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
    //testToken(&parser);
    testCompile(&parser);
    return 0;
}