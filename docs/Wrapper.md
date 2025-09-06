### Export Wrapper - 实现 FFI 的必经之路

假设有一程序如下

```
func test(a: int, b: int) : int {
    return a + b
}
```

在经过编译后，所生成的函数签名应为 `YoiIntegerObject* 0_test#int#int(YoiIntegerObject *a, YoiIntegerObject *b)`。显而易见，经过了 `name mangling` 函数名称显然不能直接被外部库使用 `extern "C"` 直接使用，更不用提 `Everything is object` 设计理念之下参数和返回值是 `YoiIntegerObject` 指针这种问题了。

如何使 hoshi-lang 程序被外界的动态库以一种更友好的方式，在不要求外部程序主动使用 hoshi-lang 对象管理逻辑的情况下调用 hoshi-lang 接口呢？这时候就要用到 `export` 关键字了。

### What is export wrapper?

`export-as` 是用于实现 `hoshi-lang` 导出函数的关键字。它接受一个 普通函数、实例化模板函数、普通结构体、实例化结构体 作为输入，然后在 llvmCodegen 过程中将输入转换为 `cdecl` 标准的函数或结构体。

假设我有如下代码

```
func test(a: int, b: int) : int {
    return a + b
}

export test as test
```

此时，原 test 函数的 name mangling 不会被改变，对应地，一个签名名为 `int test(int a, int b)` 的 `wrapper` 函数在 llvmCodegen 阶段被生成，它将正常的参数接收并调用运行时函数转换为 `YoiIntegerObject *`，增加引用计数后传入 原 test 函数，接受返回值，并解引用 YoiIntegerObject* 对象，减少引用计数后返回 `int` 类型返回值。

### When there's a `export wrapper`, there must be a `import wrapper`

与 `export wrapper` 相反，hoshi-lang 也存在用于声明外部函数的 wrapper，此即 `import wrapper`。

假设我有如下静态库 `libfoobar.so` 中的如下函数签名

```c
FILE* fopen(const char* path, const char *flags)
```

如何调用这个函数呢？有如下代码：

```
import fopen(path: str, flags: str) : int from "libfoobar.so"
```

其中由于 hoshi-lang 不存在指针的语言设计，FILE* 被替换为了等长的 `int` 也就是 `i64`。

### 复杂类型的 export 和 import

对于基础类型，hoshi-lang 会自动进行 wrapper 的生成，对于复杂类型如 structObject 需要用户声明 export struct 后才会进行对应 cdecl 的 struct wrapper 创建。

如下

```
struct Point {
    a: int,
    b: int,
    constructor(a: int, b: int)
}

export Point as Point
```

此时 Point 的空间结构如下

| Offset | Data |
| --- | --- |
| 0 ~ 7 bytes | uint64_t gc_refcount |
| 8 ~ 15 bytes | YoiIntegerObject* a |
| 16 ~ 31 bytes | YoiIntegerObject* b |

在声明 export 后，未经 name mangling 的 `Point` 在 llvmCodegen 时期被创建，与受内存管理的 `Point` 不同，此处 `Point` 的内存结构如下


| Offset | Data |
| --- | --- |
| 0 ~ 7 bytes | int a |
| 8 ~ 15 bytes | int b |

导入复杂数据结构同理。

```
import struct Point {
    a: int,
    b: int
}
```