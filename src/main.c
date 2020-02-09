#include"common.h"
#include"compiler.h"
#include"core.h"
#include"vm.h"
//#include"vm.h"
char*statement="Pudron\nexcellent and free programming language.\nusage:\n"
"compile and run:pd [file]\n"
"run pdm file:pd [file]\n"
"argument:\n"
"-m:make module\n"
"-o [name]:use output file name\n";
int run(char*fileName,char*outputName){
    PdSTD pstd=makeSTD();
    Module mod;
    char*post=getPostfix(fileName);
    if(strcmp(post,FILE_MODULE_POSTFIX)==0){
        mod=importModule(fileName);
    }else{
        mod=compileAll(fileName,pstd);
    }
    free(post);
    if(outputName!=NULL){
        exportModule(outputName,mod);
        return 0;
    }
    Unit unit=getModuleUnit(mod);
    //printCmds(unit,0);
    VM vm=newVM(fileName,pstd);
    makeSTDObject(&vm,&pstd);
    unit.gvlist=pstd.hl;
    execute(&vm,&unit);
    freeHashList(&vm,&unit,&unit.lvlist);
    freeHashList(&vm,&unit,&pstd.hl);
    Object*rt=vm.stack[--vm.stackCount];
    confirmObjectType(&vm,rt,OBJECT_INT);
    int r=rt->num;
    free(rt);
    return r;
}
int main(int argc,char**argv){
    if(argc==1){
        printf("%s",statement);
        return 0;
    }
    bool isVersion=false;
    char*fileName=NULL,*outputName=NULL;
    for(int i=1;i<argc;i++){
        if(argv[i][0]=='-'){
            if(argv[i][1]=='m'){
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
    }
    if(fileName==NULL){
        if(isVersion){
            return 0;
        }
        printf("error:expected a file.\n");
        return -1;
    }
    return run(fileName,outputName);
}