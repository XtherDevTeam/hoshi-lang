简要记一下设计思路，防止自己不记得

Callable对象的创建由编译器管理，本质上就是一个拥有 operator() 重载的 interface，按照形参类型进行名称 mangle，若形参类型一样，只做增加 implementation 的工作。
写到这里想必聪明的未来的你已经知道lambda的设计思路了。
这里就说一下 function 怎么变成这样的东西吧。
首先 interfaceImplementation 里面的 virtual method 是有具体实现的，this指针你push个null进去当实现都不用管，因为 interface 在创建的时候是转让逻辑，在回收的时候则会有空指针判断，所以在 interfaceImpl 里写上对应函数调用就行了。