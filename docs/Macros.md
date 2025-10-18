# Macros in Hoshi-lang

Hoshi-lang has a simple conditional compilation feature that works like a macro system. This allows you to include or exclude code based on flags passed to the compiler.

## Syntax

The syntax for a macro is a double-bracketed expression:

```rust
[[<key> <operator> <value>]]
```

-   `<key>`: The name of the macro.
-   `<operator>`: One of `==`, `!=`, `<`, `>`, `<=`, `>=`.
-   `<value>`: The value to compare against.

If the condition is true, the code immediately following the macro will be included in the compilation. Otherwise, it will be ignored.

## Pre-defined Macros

Hoshi-lang provides the following pre-defined macros:

-   `platform`: The operating system the compiler is running on (`"darwin"`, `"linux"`, or `"win32"`).
-   `arch`: The architecture the compiler is running on (e.g., `"x86_64"`, `"arm64"`).
-   `hoshi_feature_version`: The version of the Hoshi-lang language.
-   `hoshi_lang_commit`: The git commit hash of the Hoshi-lang compiler.

## Custom Macros

You can define your own macros using the `-D` or `--define` flag when compiling:

```shell
./hoshi_lang -D my_macro 123 my_file.hoshi
```

## Example

```rust
use runtime "runtime"

func main() : int {
    [[platform == "darwin"]] runtime.puts("Hello, world from macOS!")
    [[platform == "linux"]] runtime.puts("Hello, world from Linux!")
    [[platform == "win32"]] runtime.puts("Hello, world from Windows!")
    return 0
}
```

In this example, the appropriate `puts` call will be included based on the operating system the code is compiled on.
