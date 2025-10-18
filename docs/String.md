# String in Hoshi-lang

The `str` standard library module provides a `Str` struct for working with strings.

## `Str` Struct

The `Str` struct is a mutable string type.

### Methods

-   `constructor(literal: string)`: Creates a new `Str` from a string literal.
-   `constructor(arr: char[])`: Creates a new `Str` from a character array.
-   `constructor(capacity: unsigned)`: Creates a new `Str` with a given capacity.
-   `constructor()`: Creates an empty `Str`.
-   `push(ch: char) : none`: Appends a character to the end of the string.
-   `pop() : none`: Removes the last character from the string.
-   `size() : int`: Returns the number of characters in the string.
-   `clear() : none`: Removes all characters from the string.
-   `capacity_resize(new_capacity: unsigned) : none`: Resizes the capacity of the string.
-   `shrink_to_fit() : none`: Shrinks the capacity of the string to fit its content.
-   `startswith(prefix: Str) : bool`: Checks if the string starts with a given prefix.
-   `endswith(suffix: Str) : bool`: Checks if the string ends with a given suffix.
-   `c_str() : int`: Returns a pointer to the null-terminated C-style string.
-   `find(pattern: Str, start: int) : int`: Finds the first occurrence of a substring.
-   `find(ch: char, start: int) : int`: Finds the first occurrence of a character.
-   `substring(start: unsigned, end: unsigned) : Str`: Returns a substring.
-   `trim() : Str`: Removes leading and trailing whitespace.
-   `replace(pattern: Str, value: Str) : Str`: Replaces all occurrences of a substring.
-   `split(sep: Str) : Str[]`: Splits the string by a separator.
-   `find_all(pattern: Str) : int`: Counts the number of occurrences of a substring.
-   `reverse() : none`: Reverses the string.

### Static Methods

-   `join(strs: Str[]) : Str`: Joins an array of strings.
-   `join(args: ...) : Str`: Joins a variable number of arguments.

## `Stringable` Interface

The `Stringable` interface is implemented by types that can be converted to a `Str`.

-   `to_string() : Str`: Converts the object to a `Str`.

## `format` Function

The `format` function formats a string with a variable number of arguments.

```rust
use str "str"

func main() : int {
    let s = str.format("Hello, {}!", "world")
    return 0
}
```
