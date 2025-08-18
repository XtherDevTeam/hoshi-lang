# Array in yoi-lang

数组在 IRValueType 以 `yoi::vec<yoi::indexT> dimensions` 形式记录，若 dimensions 不为 0，则为数组类型。

数组类型的长度在编译期确定，不能更改。

出于效率考虑，对于基础类型，Array 存放的为其 raw value 而不是完整的 object。这也意味着获取元素的行为是复制并创建 object，由此无直接操作可引用一个数组中的元素。
对于非基础类型如 struct，则存放完整的 object，获取元素的行为为直接取得对应位置指针执行引用计数自增操作。

```yoi
func test() : int {
    let arr = int[10](1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    return arr[1]; // copy this element and return a new object
}
```

# Dynamic array and array

动态数组在 hoshi-lang 原生语法中指运行时确定大小的数组，其大小可以通过 lang.HasLength 接口获取（暂未实现）。其 ABI 与普通数组的 ABI 相同，拥有相同的对象头，但类型 ID 不同。

数组类型在作为参数或返回值离开函数时，其类型将自动转换为动态数组，若存在多个维度，则会展平为一维数组。

详细例子见如下：

- 1. 动态数组类型定义及跨函数传参 `examples/array-type-spec.hoshi`
- 2. 动态数组的定义和使用 `examples/new-expr.hoshi`

# Array length by `array_length`

数组长度可通过 `arr.length` 获得，这是一种由编译器支持的语法，而不是真正的 `property`。对该语法编译器行为如下，当栈上对象为动态数组时，编译器生成 `array_length` IR，并在运行时从对象头获取数组长度。而对于编译期长度确定的静态数组，则在 `IROptimizer` 阶段 `array_length` IR 被 `reduced` 并生成一个 push_integer 命令。

- 1. 见 `examples/array-length.hoshi`