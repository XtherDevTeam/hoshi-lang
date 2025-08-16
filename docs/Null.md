# The implementation of null literal in hoshi-lang

已知 `null` 是一种字面量，其意义为在栈上放置一个值为 nullptr 的指针（ `pointerObject` ）。

其预期使用如下：

```go
func main () : int {
    let x = lang.NullInterface("hello")
    // 在此处 dyn_cast 时，对 x 所持有的 type_id 进行检查时失败，此时 y 为 null 指针
    let y = dyn_cast<int>(x)
    // 此处参与运算时 y 先被 pointer_cast 命令转换为了 pointerObject 并参与比较
    if (y == null) {
        puts("validation passed")
    } else {
        puts("validation failed")
    }
}
```

## Pointer Object in hoshi-lang

Pointer object 为一种特殊的 object，其无变量头，不能被显式初始化，不能为变量类型，仅在与 null 的比较时参与运算，在 LLVM IR 中体现为一个 int64 字面量，而在 Yoi IR 层面，只支持 equal, not_equal 两种计算操作。