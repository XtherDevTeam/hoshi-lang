# Standard Library

Hoshi-lang now includes a standard library with support for common data structures.

## String (`str`)

The `str` module provides a `Str` struct for working with strings. It is available by importing `str`:

```rust
use str "str"
```

The `Str` struct has the following methods:

-   `constructor(literal: string)`: Creates a new `Str` from a string literal.
-   `constructor(arr: char[])`: Creates a new `Str` from a character array.
-   `constructor(capacity: int)`: Creates a new `Str` with a given capacity.
-   `constructor()`: Creates an empty `Str`.
-   `push(ch: char) : none`: Appends a character to the end of the string.
-   `pop() : none`: Removes the last character from the string.
-   `size() : int`: Returns the number of characters in the string.
-   `clear() : none`: Removes all characters from the string.
-   `capacity_resize(new_capacity: int) : none`: Resizes the capacity of the string.
-   `shrink_to_fit() : none`: Shrinks the capacity of the string to fit its content.
-   `startswith(prefix: Str) : bool`: Checks if the string starts with a given prefix.
-   `endswith(suffix: Str) : bool`: Checks if the string ends with a given suffix.
-   `c_str() : int`: Returns a pointer to the null-terminated C-style string.

## Vector (`vec`)

The `vec` module provides a `Vec<T>` struct for working with dynamic arrays. It is available by importing `vec`:

```rust
use vec "vec"
```

The `Vec<T>` struct has the following methods:

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
