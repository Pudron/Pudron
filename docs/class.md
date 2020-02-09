## Pudron 类
## 定义
    class 类名{
        类成员
        类方法
        func init类名(参数){
            #此方法可有可无#
        }
        func destroy类名(){
            #此方法可有可无#
        }
    }
## 示例
    class Person{
        money,handsome;
        name="MY";
        func initTest(val){
            money=val;
            handsome=true;
        }
        func destroyTest(){
            money=0;
            handsome=false;
        }
        func give(count){
            money-=count;
        }
        func operator+(count){
            money+=count;
        }
    }
    my=Person(100);
    my.give(8);
    my+=10;
## 说明
* 类成员初始化会在调用类时执行
* 类方法定义与函数相同,通过`对象.方法(参数)`来调用
* 类定义完后会有与类名相同的函数,用来创建对象
* 创建对象的同时会执行构造方法`init类名()`,其参数即为创建对象函数的参数
* 当对象被销毁时会执行方法`destroy类名()`,该方法不支持参数,不能通过直接调用该方法来销毁对象,当该对象不再被变量指向时,Pudron会自动把它销毁
* 通过定义方法`operator 操作符(参数)`来定义自定义操作符方法,该方法有且只有一个参数作为右操作数
* 通过变量`this`来访问对象本身
* 可以在类名后加`:类1,类2,...`来继承父类,可同时继承多个父类,如`class A:B,C,D{...}`
* 类定义完后,会用相应的类名变量,用于判断变量所属类,当你想知道一个变量所属的类时,通过方法`getClass()`得到其类名变量,如`if(a.getClass()=int){...}`,详见[meta](meta.md)