#include"exec.h"
void exeError(char*text){
    printf("Execute error:%s\n",text);
    exit(2);
}
int getCmdData(VM*vm,DataType dt,int val){
    char msg[100];
    if(dt==DATA_INTEGER){
        return val;
    }else if(dt==DATA_POINTER){
        return vm->data[val];
    }else if(dt==DATA_REG){
        return vm->reg[val];
    }else if(dt==DATA_REG_POINTER){
        return vm->data[vm->reg[val]];
    }else{
        sprintf(msg,"unsupport data type:%d.",val);
        exeError(msg);
    }
    return 0;
}
int*getCmdPtr(VM*vm,DataType dt,int val){
    char msg[100];
    if(dt==DATA_POINTER){
        return &vm->data[val];
    }else if(dt==DATA_REG){
        return &vm->reg[val];
    }else if(dt==DATA_REG_POINTER){
        return &vm->data[vm->reg[val]];
    }else{
        sprintf(msg,"unsupport data type:%d.",val);
        exeError(msg);
    }
    return NULL;
}
void eNop(){
    return;
}
void eMov(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)=getCmdData(vm,cmd.tb,cmd.b);
}
void eAdd(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)+=getCmdData(vm,cmd.tb,cmd.b);
}
void eSub(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)-=getCmdData(vm,cmd.tb,cmd.b);
}
void eMul(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)*=getCmdData(vm,cmd.tb,cmd.b);
}
void eDiv(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)/=getCmdData(vm,cmd.tb,cmd.b);
}
/*a的cf次方 */
int myPow(int a,int cf){
    int result=1;
    while(cf--){
        result*=a;
    }
    return result;
}
void getFloat(int val,int*num,int*dat){
    *num=val&0x1FFFFFFF;
    *dat=val>>29;
}
void eFaddOrSub(VM*vm,Cmd cmd,bool isAdd){
    int na,nb,da,db;
    int*val=getCmdPtr(vm,cmd.ta,cmd.a);
    getFloat(*val,&na,&da);
    getFloat(getCmdData(vm,cmd.tb,cmd.b),&nb,&db);
    if(da>db){
        na*=myPow(10,da-db);
        da=db;
    }else{
        nb*=myPow(10,db-da);
    }
    if(isAdd){
        na=na+nb;
    }else{
        na=na-nb;
    }
    *val=(na&0x1FFFFFFF)|(da<<29);
}
void eFmulOrDiv(VM*vm,Cmd cmd,bool isMul){
    int na,nb,da,db;
    int*val=getCmdPtr(vm,cmd.ta,cmd.a);
    getFloat(*val,&na,&da);
    getFloat(getCmdData(vm,cmd.tb,cmd.b),&nb,&db);
    if(isMul){
        *val=((na*nb)&0x1FFFFFFF)|((da+db)<<29);
    }else{
        *val=((na/nb)&0x1FFFFFFF)|((da-db)<<29);
    }
}
void eSubs(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    *a=-(*a);
}
void eEqual(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    if(*a==getCmdData(vm,cmd.tb,cmd.b)){
        *a=1;
    }else{
        *a=0;
    }
}
void eJmp(VM*vm,Cmd cmd){
    vm->ptr=vm->ptr+getCmdData(vm,cmd.ta,cmd.a)-1;
}
void eJmpc(VM*vm,Cmd cmd){
    if(vm->reg[REG_CF]==0){
        vm->ptr=vm->ptr+getCmdData(vm,cmd.ta,cmd.a)-1;
    }
}
void ePush(VM*vm,Cmd cmd){
    LIST_ADD(vm->stack,int,getCmdData(vm,cmd.ta,cmd.a));
}
void ePop(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)=vm->stack.vals[vm->stack.count-1];
    LIST_SUB(vm->stack,int);
}
void ePopt(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)=vm->stack.vals[vm->stack.count-1-getCmdData(vm,cmd.tb,cmd.b)];
}
void eSfree(VM*vm,Cmd cmd){
    vm->stack.count-=getCmdData(vm,cmd.ta,cmd.a);
    vm->stack.size=(vm->stack.count/LIST_UNIT_SIZE+1)*LIST_UNIT_SIZE;/*可能会有误差*/
    vm->stack.vals=realloc(vm->stack.vals,vm->stack.size*sizeof(int));
}
void eCand(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    int b=getCmdData(vm,cmd.tb,cmd.b);
    if(*a && b){
        *a=1;
    }else{
        *a=0;
    }
}
void eCor(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    int b=getCmdData(vm,cmd.tb,cmd.b);
    if(*a || b){
        *a=1;
    }else{
        *a=0;
    }
}
void eAnd(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)&=getCmdData(vm,cmd.tb,cmd.b);
}
void eOr(VM*vm,Cmd cmd){
    *getCmdPtr(vm,cmd.ta,cmd.a)|=getCmdData(vm,cmd.tb,cmd.b);
}
void eInvert(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    *a=!(*a);
}
void eGthan(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    if(*a>getCmdData(vm,cmd.tb,cmd.b)){
        *a=1;
    }else{
        *a=0;
    }
}
void eGthanEqual(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    if(*a>=getCmdData(vm,cmd.tb,cmd.b)){
        *a=1;
    }else{
        *a=0;
    }
}
void eLthan(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    if(*a<getCmdData(vm,cmd.tb,cmd.b)){
        *a=1;
    }else{
        *a=0;
    }
}
void eLthanEqual(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    if(*a<=getCmdData(vm,cmd.tb,cmd.b)){
        *a=1;
    }else{
        *a=0;
    }
}
void eNotEqual(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    if(*a!=getCmdData(vm,cmd.tb,cmd.b)){
        *a=1;
    }else{
        *a=0;
    }
}
void eLeft(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    *a=(*a)<<getCmdData(vm,cmd.tb,cmd.b);
}
void eRight(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    *a=(*a)>>getCmdData(vm,cmd.tb,cmd.b);
}
void eInvert2(VM*vm,Cmd cmd){
    int*a=getCmdPtr(vm,cmd.ta,cmd.a);
    *a=~(*a);
}
void ePutc(VM*vm,Cmd cmd){
    putchar(getCmdData(vm,cmd.ta,cmd.a));
}
void initVM(VM*vm,Parser parser){
    LIST_INIT(vm->stack,int);
    vm->dataSize=parser.dataSize;
    vm->data=(int*)malloc(vm->dataSize*sizeof(int));
    vm->ptr=0;
    vm->exeClist=parser.exeClist;
    for(int i=0;i<vm->dataSize;i++){
        vm->data[i]=0;
    }
}
void execute(VM*vm){
    Cmd cmd;
    char msg[100];
    for(vm->ptr=0;vm->ptr<vm->exeClist.count;vm->ptr++){
        cmd=vm->exeClist.vals[vm->ptr];
        switch(cmd.handle){
            case HANDLE_NOP:
                eNop();
                break;
            case HANDLE_MOV:
                eMov(vm,cmd);
                break;
            case HANDLE_ADD:
                eAdd(vm,cmd);
                break;
            case HANDLE_SUB:
                eSub(vm,cmd);
                break;
            case HANDLE_SUBS:
                eSubs(vm,cmd);
                break;
            case HANDLE_MUL:
                eMul(vm,cmd);
                break;
            case HANDLE_DIV:
                eDiv(vm,cmd);
                break;
            case HANDLE_FADD:
                eFaddOrSub(vm,cmd,true);
                break;
            case HANDLE_FSUB:
                eFaddOrSub(vm,cmd,false);
                break;
            case HANDLE_FMUL:
                eFmulOrDiv(vm,cmd,true);
                break;
            case HANDLE_FDIV:
                eFmulOrDiv(vm,cmd,false);
                break;
            case HANDLE_EQUAL:
                eEqual(vm,cmd);
                break;
            case HANDLE_JMP:
                eJmp(vm,cmd);
                break;
            case HANDLE_JMPC:
                eJmpc(vm,cmd);
                break;
            case HANDLE_PUSH:
                ePush(vm,cmd);
                break;
            case HANDLE_POP:
                ePop(vm,cmd);
                break;
            case HANDLE_POPT:
                ePopt(vm,cmd);
                break;
            case HANDLE_SFREE:
                eSfree(vm,cmd);
                break;
            case HANDLE_CAND:
                eCand(vm,cmd);
                break;
            case HANDLE_COR:
                eCor(vm,cmd);
                break;
            case HANDLE_AND:
                eAnd(vm,cmd);
                break;
            case HANDLE_OR:
                eOr(vm,cmd);
                break;
            case HANDLE_GTHAN:
                eGthan(vm,cmd);
                break;
            case HANDLE_GTHAN_EQUAL:
                eGthanEqual(vm,cmd);
                break;
            case HANDLE_LTHAN:
                eLthan(vm,cmd);
                break;
            case HANDLE_LTHAN_EQUAL:
                eLthanEqual(vm,cmd);
                break;
            case HANDLE_NOT_EQUAL:
                eNotEqual(vm,cmd);
                break;
            case HANDLE_PUTC:
                ePutc(vm,cmd);
                break;
            case HANDLE_INVERT:
                eInvert(vm,cmd);
                break;
            case HANDLE_LEFT:
                eLeft(vm,cmd);
                break;
            case HANDLE_RIGHT:
                eRight(vm,cmd);
                break;
            case HANDLE_INVERT2:
                eInvert2(vm,cmd);
                break;
            default:
                sprintf(msg,"unknown command (%d)",cmd.handle);
                exeError(msg);
                break;
        }
    }
}
void dataToString(char*text,VM vm){
    char temp[50];
    text[0]='\0';
    for(int i=0;i<vm.dataSize;i++){
        sprintf(temp,"%d:%d\n",i,vm.data[i]);
        strcat(text,temp);
    }
}