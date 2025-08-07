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