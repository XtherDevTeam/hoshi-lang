# Structured Bindings in Hoshi-lang

Structured bindings in Hoshi-lang provide a convenient way to unpack the elements of an array or the fields of a struct into individual variables.

## Array Destructuring

You can unpack the elements of an array into variables using the `let [ ... ]` syntax.

```rust
let [a, b, c] = int[3](10, 20, 30)
// a is 10, b is 20, c is 30
```

You can also use `...` to ignore the rest of the elements.

```rust
let [x, y, ...] = int[5](1, 2, 3, 4, 5)
// x is 1, y is 2

let [..., z] = int[5](1, 2, 3, 4, 5)
// z is 5
```

## Struct Destructuring

You can unpack the fields of a struct into variables in the same way.

```rust
struct Point {
    x: int,
    y: int,
    constructor(x: int, y: int)
}

func main() : int {
    let [x, y] = Point(1, 2)
    // x is 1, y is 2
    return 0
}
```
