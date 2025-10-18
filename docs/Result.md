# Result Type in Hoshi-lang

The `Result<T, E>` type is a standard library feature in Hoshi-lang for functions that can return either a value or an error.

## `Result<T, E>` Struct

The `Result<T, E>` struct is an enumeration with two variants:

-   `Ok(T)`: Represents success and contains a value of type `T`.
-   `Err(E)`: Represents an error and contains a value of type `E`.

### Methods

-   `is_ok() : bool`: Returns `true` if the result is `Ok`.
-   `is_err() : bool`: Returns `true` if the result is `Err`.
-   `unwrap() : T`: Returns the value inside an `Ok`. Panics if the result is `Err`.
-   `unwrap_err() : E`: Returns the error inside an `Err`. Panics if the result is `Ok`.

### Static Methods

-   `ok(value: T) : Result<T, E>`: Creates an `Ok` result.
-   `err(error: E) : Result<T, E>`: Creates an `Err` result.

### Example

```rust
use lang "builtin"
use str "str"

func test(value: int) : lang.Result<int, str.Str> {
    if (value == 0xe1751aff) {
        return lang.Result<int, str.Str>.ok(value)
    } else {
        return lang.Result<int, str.Str>.err(str.Str("Error: invalid value"))
    }
}

func main() : int {
    let result = test(0xe1751aff)
    if (result.is_ok()) {
        let x = result.unwrap()
    } else {
        let err = result.unwrap_err()
    }
    return 0
}
```
