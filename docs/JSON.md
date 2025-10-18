# JSON in Hoshi-lang

The `json` standard library module provides functionality for parsing JSON data.

## `parse` Function

The `parse` function takes a `str.Str` object containing JSON data and returns a `lang.Result<JSONValue, str.Str>`.

-   If parsing is successful, the `Result` will be an `Ok` containing a `JSONValue`.
-   If parsing fails, the `Result` will be an `Err` containing an error message.

## `JSONValue` Interface

The `JSONValue` interface is a marker interface for the different types of JSON values:

-   `str.Str`: For JSON strings.
-   `Numeric`: For JSON numbers.
-   `JSONNull`: For JSON `null`.
-   `JSONDict`: For JSON objects (a `hashMap.HashMap<str.Str, JSONValue>`)
-   `JSONList`: For JSON arrays (a `vec.Vec<JSONValue>`)

## Example

```rust
use json "json"
use str "str"

func main() : int {
    let s = str.Str("{\"a\": 1, \"b\": \"hello\"}")
    let result = json.parse(s)
    if (result.is_ok()) {
        let json_obj = result.unwrap() as json.JSONDict
        let a = json_obj["a"] as json.Numeric
        let b = json_obj["b"] as str.Str
    }
    return 0
}
```
