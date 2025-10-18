# Type Aliases in Hoshi-lang

The `alias` keyword in Hoshi-lang allows you to create a new name for an existing type. This can be useful for creating shorter, more descriptive names for complex types.

## Syntax

```rust
alias NewTypeName = ExistingType
```

## Example

```rust
use hashMap "hashMap"
use str "str"

alias Map = hashMap.HashMap<str.Str, int>

func main() : int {
    let m : Map = Map()
    m["Hello"] = 0xe1751aff
    return 0
}
```

In this example, `Map` is created as an alias for `hashMap.HashMap<str.Str, int>`. This makes the code more readable and easier to write.
