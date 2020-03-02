/*暂时没什么用处
FUNC_DEF(change_charset)
    Object*st=loadVar(vm,unit,"text");
    Object*toc=loadVar(vm,unit,"toCode");
    Object*foc=loadVar(vm,unit,"fromCode");
    char*toCode=wstrtostr(toc->str);
    char*fromCode=wstrtostr(foc->str);
    char*fstr=wstrtostr(st->str);
    if(toCode==NULL || fromCode==NULL || fstr==NULL){
        vmError(vm,L"changeCharset:%s.",strerror(errno));
    }
    iconv_t cd=iconv_open(toCode,fromCode);
    if(cd==(iconv_t)-1){
        vmError(vm,L"changeCharset:%s.",strerror(errno));
    }
    size_t flen=strlen(fstr),dlen=flen*2;
    char*dstr=(char*)memManage(NULL,(dlen+1)*sizeof(char));
    char*dptr=dstr,*fptr=fstr;
    if(iconv(cd,&fptr,&flen,&dptr,&dlen)==(size_t)-1){
        vmError(vm,L"changeCharset:%s.",strerror(errno));
    }
    iconv_close(cd);
    wchar_t*wstr=strtowstr(dstr);
    Object*rt=newStringObject(vm,wstr);
    free(toCode);
    free(fromCode);
    free(fstr);
    free(dstr);
    reduceRef(vm,unit,foc);
    reduceRef(vm,unit,toc);
    reduceRef(vm,unit,st);
    PUSH(rt);
    return;
FUNC_END()*/