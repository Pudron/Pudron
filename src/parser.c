#include"parser.h"
const int SYMBOL_COUNT=36;
const int KEYWORD_COUNT=22;
const TokenSymbol symbolList[]={
    /*多字符运算符放前面*/
    {TOKEN_LEFT_EQUAL,"<<=",3},
    {TOKEN_RIGHT_EQUAL,">>=",3},
    //{TOKEN_DOUBLE_ADD,"++",2},
    //{TOKEN_DOUBLE_SUB,"--",2},
    {TOKEN_GTHAN_EQUAL,">=",2},
    {TOKEN_LTHAN_EQUAL,"<=",2},
    {TOKEN_NOT_EQUAL,"!=",2},
    {TOKEN_LEFT,"<<",2},
    {TOKEN_RIGHT,">>",2},
    {TOKEN_ADD_EQUAL,"+=",2},
    {TOKEN_SUB_EQUAL,"-=",2},
    {TOKEN_MUL_EQUAL,"*=",2},
    {TOKEN_DIV_EQUAL,"/=",2},
    {TOKEN_AND_EQUAL,"&=",2},
    {TOKEN_OR_EQUAL,"|=",2},
    {TOKEN_PERCENT_EQUAL,"%=",2},
    {TOKEN_ADD,"+",1},
    {TOKEN_SUB,"-",1},
    {TOKEN_MUL,"*",1},
    {TOKEN_DIV,"/",1},
    {TOKEN_EXCL,"!",1},
    {TOKEN_EQUAL,"=",1},
    {TOKEN_POINT,".",1},
    {TOKEN_COMMA,",",1},
    {TOKEN_SEMI,";",1},
    {TOKEN_PARE1,"(",1},
    {TOKEN_PARE2,")",1},
    {TOKEN_BRACE1,"{",1},
    {TOKEN_BRACE2,"}",1},
    {TOKEN_BRACKET1,"[",1},
    {TOKEN_BRACKET2,"]",1},
    {TOKEN_GTHAN,">",1},
    {TOKEN_LTHAN,"<",1},
    {TOKEN_INVERT,"~",1},
    {TOKEN_AND,"&",1},
    {TOKEN_OR,"|",1},
    {TOKEN_PERCENT,"%",1},
    {TOKEN_COLON,":",1}
};
const Keyword keywordList[]={
    {TOKEN_FUNC,"func","函数"},
    {TOKEN_WHILE,"while","当"},
    {TOKEN_FOR,"for","循环"},
    {TOKEN_BREAK,"break","结束"},
    {TOKEN_RETURN,"return","返回"},
    {TOKEN_IF,"if","如果"},
    {TOKEN_ELIF,"elif","若"},
    {TOKEN_ELSE,"else","否则"},
    {TOKEN_CAND,"and","且"},
    {TOKEN_COR,"or","或"},
    {TOKEN_CLASS,"class","类"},
    {TOKEN_CAND,"and","且"},
    {TOKEN_COR,"or","或"},
    {TOKEN_IMPORT,"import","引用"},
    {TOKEN_INCLUDE,"include","包括"},
    {TOKEN_TRUE,"true","真"},
    {TOKEN_FALSE,"false","假"},
    {TOKEN_DO,"do","做"},
    {TOKEN_CONTINUE,"continue","继续"},
    {TOKEN_TRY,"try","尝试"},
    {TOKEN_CATCH,"catch","捕获"}
};
/*Token*/
Token getToken(Parser*parser){
    char word[MAX_STRING];
    Msg msg;
    Token token;
    bool isFound;
    msg.fileName=parser->fileName;
    msg.code=parser->code;
    char c=parser->code[parser->ptr];
    while(c==' ' || c=='	' || c=='\n' || c=='\r' || c=='#'){
        parser->column++;
        if(c=='\n'){
            parser->line++;
            parser->column=1;
        }
        if(c=='#'){
            /*跳过注释*/
            msg.column=parser->column;
            msg.line=parser->line;
            msg.start=parser->ptr;
            c=parser->code[++parser->ptr];
            /*单行注释*/
            if(c=='#'){
                while(c!='\n' && c!='\0'){
                    parser->column++;
                    c=parser->code[++parser->ptr];
                }
                /*交给下一次循环处理*/
                parser->ptr--;
            }else{
                while(c!='#'){
                    if(c=='\0'){
                        msg.type=MSG_ERROR;
                        if(parser->ptr-msg.start<20){
                            msg.end=parser->ptr;
                        }else{
                            msg.end=msg.start+20;
                        }
                        strcpy(msg.text,"unterminated comment.");
                        reportMsg(msg);
                    }
                    parser->column++;
                    if(c=='\n'){
                        parser->line++;
                        parser->column=1;
                    }
                    c=parser->code[++parser->ptr];
                }
            }
        }
        c=parser->code[++parser->ptr];
    }
    msg.start=parser->ptr;
    token.start=parser->ptr;
    token.line=parser->line;
    token.column=parser->column;
    if(c=='\0'){
        token.type=TOKEN_END;
        token.end=parser->ptr;
        return token;
    }
    if(c>='0' && c<='9'){
        int num=0,dat=0;
        msg.start=parser->ptr;
        while(c>='0' && c<='9'){
            num=num*10+c-'0';
            parser->column++;
            c=parser->code[++parser->ptr];
        }
        token.type=TOKEN_INTEGER;
        token.num=num;
        if(c=='.'){
            c=parser->code[++parser->ptr];
            while(c>='0' && c<='9'){
                dat++;
                num=num*10+c-'0';
                parser->column++;
                c=parser->code[++parser->ptr];
            }
            if(dat==0){
                parser->ptr--;
            }else{
                token.type=TOKEN_DOUBLE;
                token.numd=num;
                for(int i=0;i<dat;i++){
                    token.numd/=10;
                }
            }
        }
        token.end=parser->ptr;
        return token;
    }else if((c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_' || (c>='0' && c<='9')){
        int i=0;
        msg.line=parser->line;
        msg.column=parser->column;
        msg.start=parser->ptr;
        for(i=0;(c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_' || (c>='0' && c<='9');i++){
            if(i>=MAX_STRING-1){
                msg.type=MSG_ERROR;
                msg.end=msg.start+99;
                strcpy(msg.text,"too long word.");
                reportMsg(msg);
            }
            word[i]=c;
            parser->column++;
            c=parser->code[++parser->ptr];
        }
        word[i]='\0';
        parser->column+=i;
        token.word=(char*)malloc(i+1);        strcpy(token.word,word);
        token.type=TOKEN_WORD;
        token.end=parser->ptr;
    }else if(c=='\''){
        int i=0;
        msg.line=parser->line;
        msg.column=parser->column;
        msg.start=parser->ptr;
        msg.type=MSG_ERROR;
        c=parser->code[++parser->ptr];
        for(i=0;c!='\'';i++){
            if(c=='\0'){
                if(parser->ptr>msg.start+99){
                    msg.end=msg.start+99;
                }else{
                    msg.end=parser->ptr;
                }
                strcpy(msg.text,"expected \' after the name.");
                reportMsg(msg);
            }
            if(i>=MAX_STRING-1){
                msg.end=msg.start+99;
                strcpy(msg.text,"the name is too long.");
                reportMsg(msg);
            }
            if(c=='\n'){
                parser->line++;
                parser->column=1;
                i--;
                c=parser->code[++parser->ptr];
                continue;
            }
            word[i]=c;
            c=parser->code[++parser->ptr];
            parser->column++;
        }
        word[i]='\0';
        token.word=(char*)malloc(i+1);
        strcpy(token.word,word);
        token.type=TOKEN_WORD;
        token.end=parser->ptr;
    }else if(c=='\"'){
        int i=0;
        msg.line=parser->line;
        msg.column=parser->column;
        msg.start=parser->ptr;
        msg.type=MSG_ERROR;
        c=parser->code[++parser->ptr];
        for(i=0;c!='\"';i++){
            if(c=='\0'){
                if(parser->ptr-msg.start>20){
                    msg.end=msg.start+20;
                }else{
                    msg.end=parser->ptr;
                }
                strcpy(msg.text,"expected \" after the string.");
                reportMsg(msg);
            }
            if(i>=MAX_STRING-1){
                msg.end=parser->ptr;
                strcpy(msg.text,"the string is too long.");
                reportMsg(msg);
            }
            if(c=='\n'){
                parser->line++;
                parser->column=1;
                i--;
                c=parser->code[++parser->ptr];
                continue;
            }else if(c=='\r'){
                i--;
                c=parser->code[++parser->ptr];
            }else if(c=='\\'){
                switch(parser->code[parser->ptr+1]){
                    case 'n':
                        c='\n';
                        parser->ptr++;
                        break;
                    case '\\':
                        c='\\';
                        parser->ptr++;
                        break;
                    case '0':
                        c='\0';
                        parser->ptr++;
                        break;
                    case '\'':
                        c='\'';
                        parser->ptr++;
                        break;
                    case '\"':
                        c='\"';
                        parser->ptr++;
                        break;
                    case 'r':
                        c='\r';
                        parser->ptr++;
                        break;
                    case 't':
                        c='\t';
                        parser->ptr++;
                        break;
                    case 'v':
                        c='\v';
                        parser->ptr++;
                        break;
                    case 'a':
                        c='\a';
                        parser->ptr++;
                        break;
                    case 'b':
                        c='\b';
                        parser->ptr++;
                        break;
                    case 'f':
                        c='\f';
                        parser->ptr++;
                        break;
                    default:
                        break;
                }
            }
            word[i]=c;
            c=parser->code[++parser->ptr];
            parser->column++;
        }
        parser->ptr++;
        word[i]='\0';
        token.str=strtowstr(word);
        if(token.str==NULL){
            msg.end=parser->ptr;
            strcpy(msg.text,strerror(errno));
            reportMsg(msg);
        }
        token.type=TOKEN_STRING;
        token.end=parser->ptr;
        return token;
    }else{
        TokenSymbol symbol;
        for(int i=0;i<SYMBOL_COUNT;i++){
            symbol=symbolList[i];
            isFound=false;
            for(int i2=0;i2<symbol.len;i2++){
                if(symbol.text[i2]==parser->code[parser->ptr+i2]){
                    isFound=true;
                }else{
                    isFound=false;
                    break;
                }
            }
            if(isFound){
                token.type=symbol.type;
                token.end=parser->ptr;
                parser->ptr+=symbol.len;
                parser->column+=symbol.len;
                return token;
            }
        }
        msg.column=parser->column;
        msg.line=parser->line;
        msg.end=parser->ptr;
        msg.type=MSG_ERROR;
        sprintf(msg.text,"unknown charactor \"%c\"(%d).",c,c);
        reportMsg(msg);
    }
    /*查找关键字*/
    if(token.type==TOKEN_WORD){
        for(int i=0;i<KEYWORD_COUNT;i++){
            if(strcmp(token.word,keywordList[i].name)==0 || strcmp(token.word,keywordList[i].other)==0){
                token.type=keywordList[i].type;
                free(token.word);
                token.word='\0';
                return token;
            }
        }
    }
    return token;
}
void getAllToken(Parser*parser){
    Token token;
    do{
        token=getToken(parser);
        LIST_ADD(parser->tokenList,Token,token)
    }while(token.type!=TOKEN_END);
    parser->ptr=0;
    parser->line=1;
    parser->column=1;
}
Token nextToken(Parser*parser){
    Msg msg;
    if(parser->curToken+1>=parser->tokenList.count){
        msg.code=parser->code;
        msg.fileName=parser->fileName;
        msg.line=parser->line;
        msg.column=parser->column;
        msg.start=0;
        msg.end=0;
        msg.type=MSG_ERROR;
        strcpy(msg.text,"next token overflow");
        reportMsg(msg);
    }
    Token token=parser->tokenList.vals[++parser->curToken];
    parser->ptr=token.end;
    parser->line=token.line;
    parser->column=token.column;
    return token;
}
Token lastToken(Parser*parser){
    Msg msg;
    if(parser->curToken<0){
        msg.code=parser->code;
        msg.fileName=parser->fileName;
        msg.line=parser->line;
        msg.column=parser->column;
        msg.start=0;
        msg.end=0;
        msg.type=MSG_ERROR;
        strcpy(msg.text,"last token overflow");
        reportMsg(msg);
    }
    Token token;
    parser->curToken--;
    if(parser->curToken>=0){
        token=parser->tokenList.vals[parser->curToken];
        parser->ptr=token.end;
        parser->line=token.line;
        parser->column=token.column;
    }
    return token;
}
Token matchToken(Parser*parser,Tokentype et,char*str,int start){
    Token token=nextToken(parser);
    Msg msg;
    if(token.type!=et){
        msg.fileName=parser->fileName;
        msg.code=parser->code;
        msg.type=MSG_ERROR;
        msg.line=parser->line;
        msg.column=parser->column;
        msg.start=start;
        msg.end=parser->ptr;
        sprintf(msg.text,"expected %s.",str);
        reportMsg(msg);
    }
    return token;
}
/*Token end*/
Parser newParser(char*fileName){
    Parser parser;
    parser.fileName=fileName;
    parser.column=1;
    parser.line=1;
    parser.ptr=0;
    parser.curToken=-1;
    parser.code=readTextFile(fileName);
    if(parser.code==NULL){
        exit(-1);
    }
    LIST_INIT(parser.tokenList)
    return parser;
}