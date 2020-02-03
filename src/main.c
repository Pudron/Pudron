#include"common.h"
#include"compiler.h"
#include"core.h"
//#include"vm.h"
char*statement="Pudron\nexcellent and free programming language.\nusage:\n"
"compile and run:pd [file]\n"
"run pdm file:pd [file]\n"
"argument:\n"
"-m:make module\n"
"-o [name]:use output file name\n";
void run(char*fileName){
    PdSTD pstd=makeSTD();
    Module mod=compileAll(fileName,pstd);
    Unit unit=getModuleUnit(mod);
    printCmds(unit,0);
}
int main(int argc,char**argv){
    if(argc==1){
        printf("%s",statement);
        return 0;
    }
    bool isLib=false,isVersion=false;
    char*fileName=NULL,*outputName=NULL;
    /*for(int i=1;i<argc;i++){
        if(argv[i][0]=='-'){
            if(argv[i][1]=='m'){
                isLib=true;
            }else if(argv[i][1]=='o'){
                i++;
                if(i>=argc){
                    printf("error:expected output file name.\n");
                    return -1;
                }
                outputName=argv[i];
            }else if(argv[i][1]=='v'){
                printf("version%d\n",VERSION);
                isVersion=true;
            }else{
                printf("error:unknown argument \"%s\".\n",argv[i]);
                return -1;
            }
        }else{
            if(fileName!=NULL){
                printf("error:can only compile one file.\n");
                return -1;
            }
            fileName=argv[i];
        }
    }*/
    fileName=argv[1];
    if(fileName==NULL){
        if(isVersion){
            return 0;
        }
        printf("error:expected a file.\n");
        return -1;
    }
    if(outputName!=NULL && !isLib){
        printf("error:nothing to output.\n");
        return -1;
    }
    /*char*post=getPostfix(fileName);
    if(strcmp(post,FILE_LIB_POSTFIX)==0){
        if(isLib){
            printf("warning:can not make library again.\n");
        }
        direct(fileName);
    }else{
        run(fileName,isLib,outputName);
    }
    free(post);*/
    run(fileName);
    return 0;
}