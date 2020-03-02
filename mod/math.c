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

#include<math.h>
#include"pdex.h"
#define FUNC_DOUBLE_DEF_1(func_name,c_name) \
    PD_FUNC_DEF(func_name){\
        if(PD_ARG_COUNT!=1){\
            PD_ERROR(PERROR_ARGUMENT,L"the math function need 1 argument.");\
        }\
        if(PD_ARG(0).type==PVAL_INT){\
            PD_RETURN_DOUBLE(c_name((double)PD_ARG(0).num));\
        }else if(PD_ARG(0).type==PVAL_DOUBLE){\
            PD_RETURN_DOUBLE(c_name(PD_ARG(0).numd));\
        }else{\
            PD_ERROR(PERROR_ARGUMENT,L"the math function need a double or int type argument.");\
        }\
    }

#define FUNC_DOUBLE_DEF_2(func_name,c_name) \
    PD_FUNC_DEF(func_name){\
        if(PD_ARG_COUNT!=2){\
            PD_ERROR(PERROR_ARGUMENT,L"the math function need 2 argument.");\
        }\
        if(PD_ARG(0).type==PVAL_INT){\
            if(PD_ARG(1).type==PVAL_INT){\
                PD_RETURN_DOUBLE(c_name((double)PD_ARG(0).num,(double)PD_ARG(1).num));\
            }else if(PD_ARG(1).type==PVAL_DOUBLE){\
                PD_RETURN_DOUBLE(c_name((double)PD_ARG(0).num,PD_ARG(1).numd));\
            }else{\
                PD_ERROR(PERROR_ARGUMENT,L"the math function need a double or int type argument.");\
            }\
        }else if(PD_ARG(0).type==PVAL_DOUBLE){\
            if(PD_ARG(1).type==PVAL_INT){\
                PD_RETURN_DOUBLE(c_name(PD_ARG(0).numd,(double)PD_ARG(1).num));\
            }else if(PD_ARG(1).type==PVAL_DOUBLE){\
                PD_RETURN_DOUBLE(c_name(PD_ARG(0).numd,PD_ARG(1).numd));\
            }else{\
                PD_ERROR(PERROR_ARGUMENT,L"the math function need a double or int type argument.");\
            }\
        }else{\
            PD_ERROR(PERROR_ARGUMENT,L"the math function need a double or int type argument.");\
        }\
    }

FUNC_DOUBLE_DEF_1(msqrt,sqrt)
FUNC_DOUBLE_DEF_2(mpow,pow)
FUNC_DOUBLE_DEF_1(macos,acos)
FUNC_DOUBLE_DEF_1(masin,asin)
FUNC_DOUBLE_DEF_1(matan,atan)
FUNC_DOUBLE_DEF_2(matan2,atan2)
FUNC_DOUBLE_DEF_1(mcos,cos)
FUNC_DOUBLE_DEF_1(mcosh,cosh)
FUNC_DOUBLE_DEF_1(msin,sin)
FUNC_DOUBLE_DEF_1(msinh,sinh)
FUNC_DOUBLE_DEF_1(mtan,tan)
FUNC_DOUBLE_DEF_1(mtanh,tanh)
FUNC_DOUBLE_DEF_1(mexp,exp)
FUNC_DOUBLE_DEF_1(mlog,log)
FUNC_DOUBLE_DEF_1(mlog10,log10)
FUNC_DOUBLE_DEF_1(mceil,ceil)
FUNC_DOUBLE_DEF_1(mabs,fabs)
FUNC_DOUBLE_DEF_1(mfloor,floor)
FUNC_DOUBLE_DEF_2(mmod,fmod)