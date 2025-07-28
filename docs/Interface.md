# 接口设计

接口在中间代码分为 `InterfaceImplmentation` 和 `InterfaceInstance`

其中，InterfaceImplmentation 指不同 struct 对 interface 的实现，与类型强相关，而 `InterfaceInstance` 则是 struct 被实例化的接口，已与 struct 类型无关。

InterfaceInstance 为一个特殊化的 struct 对象，由一个指向对应 struct 的指针和一族方法指针组成。

不同 struct 的同一 interface，其实例化后的 instance 仍可通用，其 `gc_refcount_increase` 和 `gc_refcount_decrese` 也为函数指针，指向对应实现的处理方法，无需担心未被正常释放。

# new 语句设计

创建的对象不论 object 和 interface 统统分配在堆上，由运行时函数统一开辟空间，对于 new object 的过程大致如下：

1. `new_object` 或 `new_object_extern` 带有对应 struct index 被调用，llvmCodegen 根据 struct 的结构，生成开辟空间的 IR code，并压入对应 struct object 到栈上
2. 调用 `invoke` 或 `invoke_extern` 执行 constructor 初始化函数。（注意空指针的问题）