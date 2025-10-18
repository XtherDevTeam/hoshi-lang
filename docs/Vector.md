# Vector in Hoshi-lang

The `vec` standard library module provides a `Vec<T>` struct for working with dynamic arrays.

## `Vec<T>` Struct

The `Vec<T>` struct is a generic dynamic array implementation.

### Methods

-   `constructor(capacity: int)`: Creates a new `Vec` with a given capacity.
-   `constructor(data: T[])`: Creates a new `Vec` from an array.
-   `constructor(data: T[], capacity: int)`: Creates a new `Vec` from an array with a given capacity.
-   `constructor()`: Creates an empty `Vec`.
-   `push(x: T) : none`: Appends an element to the end of the vector.
-   `pop() : none`: Removes the last element from the vector.
-   `size() : int`: Returns the number of elements in the vector.
-   `clear() : none`: Removes all elements from the vector.
-   `capacity_resize(new_capacity: int) : none`: Resizes the capacity of the vector.
-   `shrink_to_fit() : none`: Shrinks the capacity of the vector to fit its content.

### Example

```rust
use vec "vec"

func main() : int {
    let v = vec.Vec<int>()
    v.push(1)
    v.push(2)
    v.push(3)
    return 0
}
```
