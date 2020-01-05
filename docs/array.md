# Pudron 数组
定义格式：`{表达式1,表达式2,...}`  
访问数组成员格式：`数组变量[索引]`,索引以0为起点  
示例：

    array={1,2,3};
    element=array[0];

你也可以创建一个空的数组，Pudron会为其分配内存，一般用于动态分配元素数量(但建议用[List](list.md)类替代)  
[函数](function.md)中的`argVar`也是数组。  
字符串不是严格的数组，但也能和数组一样访问字符。　　
数组的元素数量可以用`getVarCount()`方法获得。  
若要重新分配数组元素数量，可以用`resizeVar(size)`方法。  
下面代码将实现为动态添加一个元素：

    func expand(array,element){
        array.resizeVar(array.getVarCount()+1);
        array[array.getVarCount()-1]=element
    }
    array={};
    expand(array,123);

以上方法属于[meta](meta.md)。  
关于类的说明，见[类](class.md)。