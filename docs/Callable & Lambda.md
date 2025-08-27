# Callable Objects & Lambda Expressions

Hoshi-lang supports callable objects and lambda expressions, allowing for more flexible and functional programming styles.

## 1. Callable Objects and `func` type

In Hoshi-lang, any object that overloads the `operator()` is considered a callable object. These objects can be passed around and invoked like functions.

The `func` keyword provides a way to define the type of a callable object.

### `func` Type Syntax

The syntax for a `func` type is as follows:

```
func(parameter_type_1, parameter_type_2, ...) : return_type
```

For example, a function type that takes two `int`s and returns an `int` would be:

```
func(int, int) : int
```

## 2. Lambda Expressions

Lambda expressions provide a concise way to create anonymous functions.

### Syntax

The syntax for a lambda expression is:

```
func[capture_list](parameters) : return_type {
    // body
}
```

- `capture_list`: A comma-separated list of variables from the enclosing scope to be captured by the lambda. Captured variables are accessed as members of `this` within the lambda's body.
- `parameters`: The parameters of the lambda function.
- `return_type`: The return type of the lambda function.
- `body`: The code to be executed when the lambda is called.

### Example

Here is an example of how to define and use a lambda expression:

```rust
use lang "builtin"
import puts(str: ptr) : int32 from "builtin"

// A function that accepts a callable object as a parameter
func test_lambda(x: int, y: int, f: func (int, int) : int) : int {
    return f(x, y)
}

func main() : int {
    let salt = 114514

    // A lambda that captures 'salt' from its environment
    let closure = func[salt] (x: int, y: int) : int {
        return x + y + this.salt
    }

    // Pass the lambda to the function
    let result = test_lambda(2, 3, closure)

    if (result != 114519) {
        puts("lambda validation failed")
        return 1
    }
    puts("lambda validation passed")
    return 0
}
```

In this example, the lambda captures the `salt` variable. Inside the lambda, `this.salt` is used to access the captured value. The lambda is then passed to the `test_lambda` function, which invokes it.

## 3. Under the Hood: Implementation Details

The implementation of callable objects and lambdas relies on interfaces and compiler-generated code.

- **Callable Objects as Interfaces**: A `func` type is essentially an interface with an `operator()` method. The compiler mangles the interface name based on the parameter types to ensure type safety and allow for reuse. If a callable interface with the same parameter types already exists, the compiler will simply add a new implementation for it.

- **Lambdas as Syntactic Sugar**: A lambda expression is syntactic sugar that directs the compiler to create a struct. The captured variables become fields of this struct. The lambda's body becomes the `operator()` method of the struct. The compiler then generates an implementation of the corresponding callable interface for this new struct.
