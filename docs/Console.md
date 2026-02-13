# Console I/O in Hoshi-lang

Hoshi-lang provides a `console` standard library module for interacting with the console.

## `print` Function

The `print` function is used to print text to the console.

### Overloads

-   `print(s: str.Str) : none`: Prints a `Str` object to the console.
-   `print(args: ...str.Stringable) : none`: Prints a variable number of arguments to the console. Each argument must implement the `Stringable` interface.

### Example

```hoshi
use console "console"
use str "str"

func main() : int {
    console.print("Hello, world!\n")
    console.print("The answer is ", 42, "\n")
    return 0
}
```

## `input` Function

The `input` function is used to read a line of text from the console.

### Overloads

-   `input() : str.Str`: Reads a line of text from the console.
-   `input(s: str.Str) : str.Str`: Prints a prompt and then reads a line of text from the console.

### Example

```hoshi
use console "console"
use str "str"

func main() : int {
    let name = console.input("What is your name? ")
    let greeting = str.format("Hello, {}!\n", name)
    console.print(greeting)
    return 0
}
```

```