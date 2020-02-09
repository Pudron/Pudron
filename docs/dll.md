# Pudron DLL
## 说明
DLL 类用于链接动态库
## 方法
* `DLL(path)`:构造函数,打开path的动态库
* `execute(type,name,arg)`:以arg为参数,执行动态库中返回值为type,名字为name的函数,返回其返回值