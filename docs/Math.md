# Math in Hoshi-lang

The `math` standard library module provides a collection of common mathematical functions.

## Functions

-   `max<T>(a: T, b: T) : T`: Returns the greater of two values.
-   `min<T>(a: T, b: T) : T`: Returns the smaller of two values.
-   `abs<T>(a: T) : T`: Returns the absolute value of a number.
-   `pow<T>(base: T, exponent: T) : T`: Returns the result of raising `base` to the power of `exponent`.
-   `sqrt<T>(a: T) : T`: Returns the square root of a number.
-   `isnan(a : deci) : bool`: Returns `true` if the given floating-point number is Not a Number (NaN).

## Example

```rust
use math "math"

func main() : int {
    let x = math.max(10, 20) // x will be 20
    let y = math.sqrt(16.0) // y will be 4.0
    return 0
}
```
