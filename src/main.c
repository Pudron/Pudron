#include"common.h"
#include"compiler.h"
char*statement="Pudron\nexcellent and free programming language.\nusage:\n"
"compile and run:pd [file]\n"
"run pdl file:pd [file]\n"
"argument:\n"
"-l:make library\n"
"-o [name]:use output file name\n";
#ifndef RELEASE
void testToken(Parser*parser){
    Token token;
    token=nextToken(parser);
    while(token.type!=TOKEN_END){
        if(token.type==TOKEN_WORD || token.type==TOKEN_STRING){
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
#endif
int main(int argc,char**argv){
    if(argc==1){
        printf("%s",statement);
        return 0;
    }
    bool isLib=false;
    char*fileName=NULL,*outputName=NULL;
    for(int i=1;i<argc;i++){
        if(argv[i][0]=='-'){
            if(argv[i][1]=='l'){
                isLib=true;
            }else if(argv[i][1]=='o'){
                i++;
                if(i>=argc){
                    printf("error:expected output file name.\n");
                    return -1;
                }
                outputName=argv[i];
            }
        }else{
            if(fileName!=NULL){
                printf("error:can only compile one file.\n");
                return -1;
            }
            fileName=argv[i];
        }
    }
    if(fileName==NULL){
        printf("error:expected a file.\n");
        return -1;
    }
    if(outputName!=NULL && !isLib){
        printf("error:nothing to output.\n");
        return -1;
    }
    char*post=getPostfix(fileName);
    if(strcmp(post,FILE_LIB_POSTFIX)==0){
        if(isLib){
            printf("warning:can not make library again.\n");
        }
        direct(fileName);
    }else{
        run(fileName,isLib,outputName);
    }
    free(post);
    return 0;
}