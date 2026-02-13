# The purposal of introducing data fields in legacy struct

With the sucessful implementation of data struct and its affiliated optimization, we significantly improved the performance of hoshi-lang over *certain* fields like scientific calculations. However, there is also limitation which it cannot incorporate methods as legacy struct does. Therefore, we acknowledge the essentiality of introducing data fields into legacy struct to further reducing redundant object allocation and improving the performance of hoshi-lang.

# The syntax of data fields in legacy struct

Suppose we have a struct `Point` defined as follows:

```hoshi
struct Point {
    x: deci,
    y: deci,
    constructor(x: deci, y: deci)
}

impl Point {
    constructor(x: deci, y: deci) {
        this.x = x
        this.y = y
    }
}
```

To employ data fields in legacy struct, we simply add `data` keyword before the field declaration:

```hoshi
struct Point {
    data x: deci,
    data y: deci,
    constructor(x: deci, y: deci)
}

impl Point {
    constructor(x: deci, y: deci) {
        this.x = x
        this.y = y
    }
}
```

# Underlying mechanisms

When compiler encounter such field declaration, it will automatically attach a Metadata named STRUCT_DATAFIELD with `true` as value into its `IRValueType`. Through checking the existence of this metadata, in `performStructNullablePass()` method, we first determine the field type. If it is a data field, we will perform the propagation of Raw value on specific field, otherwise Nullable value propagation will be performed.

With additional modification on the handling logic of `store_member` `load_member` in both IROptimizer and llvmCodegen, the fields will no longer maintain its own object reference, it would be a direct 8-bytes value storage in the struct. At the same time, it loses the ability to hold `null` value and be referenced. Instead, every assignment would be copy-constructed as Raw value, thus enabling the produced result to be further optimized by IROptimizer and llvmCodegen.