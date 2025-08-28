# Nullable Check

在 `load_xxx` `store_xxx` 命令中，为了确定减小GC计数的指针不为 `nullptr`，在此之前会对指针进行一次 ICMP_EQ 比较和跳转，由原本的 1 条指令完成的工作现在增加到4条指令。因此在 IROptimizer 之后，llvmCodegen 之前引入 Nullable Check Pass。

对一个 `Function` 单位的数据流，有如下规则：

1. 对于 `store_xxx` 指令，若右值为 `Nullable`，则 `Nullable` 属性将传递给左值，进入变量表。
    - 特别地，对于 `store_element` 命令，不作任何传递操作。
2. 对于 `load_xxx` 指令，其栈上元素属性继承自变量表或 `struct` 中元素。
    - 特别地，对于基础类型 (`isBasicType() == true`) 的数组类型 (`isDynamicArrayType() || isArrayType()`)，其 `load_element` 获得的元素，`Nullable` 属性恒为 `false`。
    - 对于 `struct` 中的成员，其 `load_member` `store_member` (获取/继承) 的 `Nullable` 均为 `true`。
3. 对于 `invoke` `invoke_virtual` 等将对应量引入外部控制流的命令，其 `Nullable` 属性无法预测，均设置为 `true`。
    - 对函数返回值、参数亦是如此。

# Raw Check

在进行计算时，小对象的创建和销毁十分频繁，为优化程序性能，在 IROptimizer 后，llvmCodegen 前引入 Raw Check Pass。

对一个 `Function` 单位的数据流，有如下规则：

1. 由 `push` 产生的基本类型常量，其 `Raw` 属性未改写前，均为 `true`。
2. 对于 `store_xxx` 命令，其 `Raw` 属性左值继承自右值。
    - 特别地，对于 `store_element` 命令，不作任何传递操作。
3. 对于 `load_xxx` 命令，其栈上 `Raw` 属性继承自左值。
    - 特别地，对于基础类型 (`isBasicType() == true`) 的数组类型 (`isDynamicArrayType() || isArrayType()`)，其 `load_element` 获得的元素，`Raw` 属性恒为 `true`。
4. 对于 `direct_assign` 命令，左值不再具有 `Raw` 属性，改写为 `false`。
5. 对于 `invoke` `invoke_virtual` 等将对应量引入外部控制流的命令，其 `Raw` 属性无法预测，均设置为 `false`。
    - 对函数返回值、参数亦是如此。

# Inter-functional call graph building and raw check

在先前的优化中，跨函数代码并未加入代码之中，而是采用保守的优化策略，视为 Nullable 变量，故引入 Call Graph 分析。

函数按从 Call Graph 的 BFS 起点开始调用 Optimizer，大体逻辑不变，加入对返回值的 Nullable 和 Raw 分析。
同时依据函数返回值的 Nullable 和 Raw 属性确定栈上返回值的 Nullable 和 Raw。

在初始态，所有的参数和返回值均被推断为 Rawable 和 Non-nullable，直到出现不符合推断信息时，删去 Raw 或添加 Nullable 

# IRValueType 属性的增加和改写

对于设置某一属性为 (真/假)，其含义为在 IRValueType 中 (增加/删除) 对该属性的定义