# Generic Programming with Templates

Hoshi-lang supports generic programming using `template`s, which allow you to write code that can work with different data types. Templates can be applied to functions, structs, and interfaces.

## Function Templates

Function templates allow you to create functions that can operate on different types.

```hoshi
func add<T>(a: T, b: T) : T {
    return a + b
}

func main() : int {
    let x = add<int>(1, 2)       // Explicit specialization
    let y = add(3.0, 4.0) // Implicit specialization
    return 0
}
```

### Implicit vs. Explicit Specialization

-   **Explicit Specialization:** You explicitly provide the template arguments (e.g., `<int>`).
-   **Implicit Specialization:** The compiler deduces the template arguments from the function's arguments.

## Struct Templates

Struct templates allow you to create generic data structures.

```hoshi
struct Container<T> {
    item: T,
    constructor(item: T)
}

impl Container<> {
    constructor(item: T) {
        this.item = item
    }
}

func main() : int {
    let c = Container<int>(123)
    return 0
}
```

## Interface Templates

Interface templates allow you to define generic contracts.

```hoshi
interface Result<T> {
    get() : T
}

struct TestStruct<T, U> {
    a: T,
    b: U,
    constructor(a: T, b: U)
}

impl TestStruct<> {
    constructor(a: T, b: U) {
        this.a = a
        this.b = b
    }
}

impl TestStruct<> : Result<T> {
    get(): T {
        return this.a
    }
}
```
