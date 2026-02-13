# Operator Overloading

Hoshi-lang allows you to define custom implementations for most of its operators on your own `struct` types. This is achieved by defining special static methods within an `impl` block.

## Overloadable Operators

The following operators can be overloaded:

| Operator | Method Name        |
|----------|--------------------|
| `+`      | `operator+`        |
| `-`      | `operator-`        |
| `*`      | `operator*`        |
| `/`      | `operator/`        |
| `%`      | `operator%`        |
| `==`     | `operator==`       |
| `!=`     | `operator!=`       |
| `<`      | `operator<`        |
| `>`      | `operator>`        |
| `<=`     | `operator<=`       |
| `>=`     | `operator>=`       |
| `&`      | `operator&`        |
| `|`      | `operator|`        |
| `^`      | `operator^`        |
| `~`      | `operator~`        |
| `<<`     | `operator<<`       |
| `>>`     | `operator>>`       |
| `++`     | `operator++`       |
| `--`     | `operator--`       |
| `()`     | `operator()`       |
| `[]`     | `operator[]`       |

## Defining Operator Overloads

To overload an operator, you define a `static` method with the corresponding `operator` name inside an `impl` block for your `struct`.

### Binary Operators

For binary operators (e.g., `+`, `-`, `*`), the overloaded method must be `static` and take two arguments, representing the left-hand side (LHS) and right-hand side (RHS) of the operation.

```hoshi
struct MyInt {
    val: int,
    constructor(val: int),
    static operator+(lhs: MyInt, rhs: MyInt) : MyInt,
    get() : int
}

impl MyInt {
    constructor(val: int) {
        this.val = val
    },
    static operator+(lhs: MyInt, rhs: MyInt) : MyInt {
        return MyInt(lhs.val + rhs.val)
    },
    get() : int {
        return this.val
    }
}

func main() : int {
    let a = MyInt(114)
    let b = MyInt(514)
    let c = a + b // c.get() will be 628
    return 0
}
```

### Unary Operators

For unary operators (e.g., `-`, `~`, `++`, `--`), the overloaded method must be `static` and take a single argument.

### Callable Objects (operator())

You can make a `struct` callable like a function by overloading the `()` operator. The method can take any number of arguments.

```hoshi
struct Callable {
    constructor(),
    operator()() : none
}

impl Callable {
    constructor() {},
    operator()() : none {
        puts("I am callable!")
    }
}

func main() : int {
    let c = Callable()
    c() // Prints "I am callable!"
    return 0
}
```

### Subscript Operator (operator[])

The subscript operator `[]` can be overloaded to provide custom indexing for your `struct`s.

-   **Getter:** A `static` method named `operator[]` that takes the object and the index as arguments.
-   **Setter:** A `static` method named `operator[]` that takes the new value, the object, and the index as arguments.

```hoshi
struct MyArray<T> {
    data: T[],
    static operator[](self: MyArray<T>, i: int) : T,
    static operator[](value: T, self: MyArray<T>, i: int) : none
}

impl MyArray<> {
    static operator[](self: MyArray<T>, i: int) : T {
        return self.data[i]
    },
    static operator[](value: T, self: MyArray<T>, i: int) : none {
        self.data[i] = value
    }
}
```
