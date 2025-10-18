# HashMap in Hoshi-lang

The `hashMap` standard library module provides a `HashMap<K, V>` struct for working with hash maps.

## `HashMap<K, V>` Struct

The `HashMap<K, V>` struct is a generic hash map implementation. The key type `K` must implement the `Hashable` interface.

### Methods

-   `constructor()`: Creates a new, empty `HashMap`.
-   `static operator[](this: HashMap<K, V>, key: K) : V`: Returns the value associated with the given key. Returns `null` if the key is not found.
-   `static operator[](value: V, this: HashMap<K, V>, key: K) : none`: Inserts or updates a key-value pair.
-   `size() : int`: Returns the number of key-value pairs in the hash map.
-   `clear() : none`: Removes all key-value pairs from the hash map.
-   `keys() : K[]`: Returns an array of all the keys in the hash map.
-   `values() : V[]`: Returns an array of all the values in the hash map.
-   `contains(key: K) : bool`: Returns `true` if the hash map contains the given key.
-   `remove(key: K) : none`: Removes a key-value pair from the hash map.

### Example

```rust
use hashMap "hashMap"
use str "str"

func main() : int {
    let map = hashMap.HashMap<str.Str, int>()
    map["Hello"] = 1
    map["World"] = 2
    return 0
}
```
