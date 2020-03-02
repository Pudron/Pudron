/***
*   Copyright (c) 2020 Pudron
*
*   This file is part of Pudron.
*
*   Pudron is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   Pudron is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
**/

#ifndef _PD_PIO_H_
#define _PD_PIO_H_
#include"common.h"
typedef struct{
    char*dat;
    int count;
    int ptr;
}Bin;
#define WRITE_LIST(dat,list,type) \
    writeInt(dat,list.count);\
    for(int i=0;i<list.count;i++){\
        write##type(dat,list.vals[i]);\
    }

/*READ_LIST()在使用前先定义变量int count;*/
#define READ_LIST(bin,list,type,typeName) \
    LIST_INIT(list)\
    count=readInt(bin);\
    for(int i=0;i<count;i++){\
        LIST_ADD(list,type,read##typeName(bin))\
    }

void writeUnit(charList*dat,Unit unit);
Unit readUnit(Bin*bin);
void writeInt(charList*dat,int num);
int readInt(Bin*bin);
void writeDouble(charList*dat,double numd);
double readDouble(Bin*bin);
void writeString(charList*dat,char*text);
char*readString(Bin*bin);
void writeWideString(charList*dat,wchar_t*wstr);
wchar_t*readWideString(Bin*bin);
void writeHashList(charList*dat,HashList hl);
HashList readHashList(Bin*bin);
bool writeFunc(charList*dat,Func func);
Func readFunc(Bin*bin);
void writeClass(charList*dat,Class class);
Class readClass(Bin*bin);

char*readTextFile(char*fileName);
bool writeTextFile(char*fileName,char*text);
void exportModule(char*fileName,Module mod);
Module importModule(char*fileName);
#endif