# Finalizers in Hoshi-lang

Finalizers in Hoshi-lang are special methods that are called just before an object is deallocated by the garbage collector. They are similar to destructors in other languages and are useful for releasing resources held by an object.

## Syntax

A finalizer is defined using the `finalizer` keyword within a `struct` definition and implemented in an `impl` block.

```rust
struct MyFile {
    file_handle: int,
    constructor(path: string),
    finalizer()
}

impl MyFile {
    constructor(path: string) {
        // open file and store handle in this.file_handle
    },
    finalizer() {
        // close file handle
    }
}
```

## Execution

When an object's reference count reaches zero, the garbage collector will call the object's `finalizer` method before freeing the object's memory. This ensures that any resources held by the object are properly released.
