# Data struct in hoshi-lang

Originally, the struct implementation in hoshi-lang is to recognize every field as a nullable field, thus causing a significant overhead in memory usage and performance degradation. Naturally, we would like to devise a new struct that preserve every field as value type, so as to maintain the memory layout of the struct and avoid the overhead of null checking, and the overhead of pointer indirection.

# Syntax

To define a data struct, it follows:

```hoshi
datastruct Test {
    s: char[256],
    t: unsigned
}
datastruct T {
    field1: int,
    field2: Test
}
```

Through this, we defined data structs named Test and T.

Note: no methods or constructors are allowed in a data struct.

# Memory Layout

For Test, it owns a memory layout of 288 bytes, with `s` taking 256 bytes and `t` taking 8 bytes.

For T, it owns a memory layout of 288 bytes, with `field1` taking 8 bytes and `field2` taking 264 bytes.

| Field | Offset | Size |
|-------|--------|------|
| Object Header | 0 | 16 |
| s | 16 | 256 |
| t | 272 | 8 |

| Field | Offset | Size |
|-------|--------|------|
| Object Header | 0 | 16 |
| field1 | 16 | 8 |
| field2 | 24 | 264 |

# Underlying mechanism

For a data struct, it only has a object header per instance, and the fields are laid out contiguously in memory. 

When attempting to acquire a field to assign as variable, a copy will be performed.

Depending on the Nullable type or not, the copy can either be a Raw value on stack or a new object on the heap.

# The integration with existing object optimization system

For a data struct, it can be recognized as a basic type just like `int`, `float`, `char`, etc. That entitled it the ability to be optimized by the existing object optimization system.

It would first be marked as a stack Raw value type until it is assigned with `null`, other Nullable value, or escape analysis determines it needs to be heap allocated, at which point it will be marked as a heap object.
