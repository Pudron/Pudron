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

#ifndef _PD_VM_H_
#define _PD_VM_H_
#include"common.h"
#include"core.h"
void execute(VM*vm,Unit*unit);
VM newVM(char*fileName,char*path,PdSTD pstd);
bool checkError(VM*vm,Unit*unit,int*ptr);
void popStack(VM*vm,Unit*unit,int num);
#endif