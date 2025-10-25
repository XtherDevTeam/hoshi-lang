# The Hoshi-lang Programming Language

## Intro

> This is one of my practice during Senior High School period. `hoshi` means both $\mathop{desire}\limits^{欲しい}$ and $\mathop{stars}\limits^{星}$ in Japanese, which also represents my silly wishes: I hope some day I would become the stars I once live up to.

Hoshi-lang is a statically-typed, general-purpose programming language with a focus on performance, safety, and modern language features. It is designed to be a simple yet powerful tool for building a wide range of applications.

This project is currently under active development and is a personal exploration into language design and implementation.

[![Checkout the Wiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/XtherDevTeam/hoshi-lang)

## Features

*   **Object-Oriented:** Hoshi-lang's OOP is based on a composition model using `interface`, `struct`, and `impl`.
*   **Generic Programming:** Supports generic programming with `template`s for both functions and structs.
*   **Operator Overloading:** Allows for custom behavior for operators like `+`, `-`, `*`, `/`, etc.
*   **Memory Safety:** Automatic Reference Counting (ARC) for memory management.
*   **Type Introspection:** Runtime type inspection with `typeid` and `interfaceof`.
*   **Dynamic Casting:** Safe casting of interface objects back to their concrete `struct` type using `dyn_cast`.
*   **Modular Programming:** Supports modules with the `use` statement.
*   **Variadic Arguments:** Functions can accept a variable number of arguments.
*   **Lambda Expressions:** Concise syntax for creating anonymous functions.
*   **Threading:** Support for multi-threaded programming.
*   **Rich Standard Library:** A growing standard library with support for strings, vectors, file I/O, and more.

## Syntax

```rust
use lang "builtin"
use io "std/io"

interface Greeter {
  say() : none
}

struct Person {
  name: string,
  constructor(name: string),
}

impl Person {
  constructor(name: string) {
    this.name = name
  }
}

impl Person : Greeter {
  say() : none {
    io.println("Hello, " + this.name)
  }
}

func main() : int {
  let p = Person("world")
  let greeter = Greeter(p)
  greeter.say()
  return 0
}
```

## Object Model and Memory Management

### Object Layout

In Hoshi-lang, **all data types are heap-allocated objects**. This includes primitives like `int`, `bool`, etc., which are "boxed" into object wrappers. The compiler and runtime manage these objects through pointers.

All objects share a common header:

| Offset | Field         | Type                 | Description                              |
|--------|---------------|----------------------|------------------------------------------|
| 0-7    | `gc_refcount` | `unsigned long long` | The object's current reference count.    |
| 8-15   | `type_id`     | `unsigned long long` | A unique ID for the object's runtime type. |

Specific object types then have their own layouts following this header.

### Interface Object Layout

Interface objects are special "shell" objects that enable polymorphism. When a `struct` instance is cast to an `interface`, a new `interfaceObject` is created with the following layout:

| Offset  | Field                               | Description                                                                                             |
|---------|-------------------------------------|---------------------------------------------------------------------------------------------------------|
| 0-15    | (Standard Header)                   | `gc_refcount` and `type_id` for the interface object itself.                                              |
| 16-23   | `this` Pointer                      | A pointer to the concrete `struct` instance that implements the interface.                              |
| 24-31   | `gc_refcount_increase` V-Ptr        | A virtual function pointer to the `increase` GC function for the concrete `struct`.                     |
| 32-39   | `gc_refcount_decrease` V-Ptr        | A virtual function pointer to the `decrease` GC function for the concrete `struct`.                     |
| 40+     | Virtual Method Pointers             | A sequence of function pointers to the concrete implementations of the interface's methods (the v-table). |

This structure is created by the compiler via the `construct_interface_impl` IR instruction.

### Garbage Collection (GC)

Memory is managed via **Automatic Reference Counting (ARC)**.

-   When an object is created or a reference is copied, its `gc_refcount` is incremented.
-   When a reference goes out of scope or is overwritten, the `gc_refcount` is decremented.
-   When the count reaches zero, the object is deallocated.

The compiler is responsible for generating calls to the appropriate `_gc_refcount_increase` and `_gc_refcount_decrease` functions for each type at the right places.

### Callable Objects & Lambda Expressions

Hoshi-lang now supports callable objects and lambda expressions, allowing for more flexible and functional programming styles.

```rust
func test_lambda(x: int, y: int, f: func (int, int) : int) : int {
    return f(x, y)
}

func main() : int {
    let salt = 114514
    let closure = func[salt] (x: int, y: int) : int {
        return x + y + this.salt
    }
    let result = test_lambda(2, 3, closure)
    return result
}
```

### Threading

Hoshi-lang now has basic support for multi-threading.

```rust
use threading "threading"
use runtime "runtime"

func worker() : none {
    runtime.puts("Hello from worker thread!")
}

func main() : int {
    let th = threading.Thread(func[] () : none {
        worker()
    })
    th.start()
    th.join()
    return 0
}
```

### Structured Bindings

You can now de-structure arrays and structs into individual variables.

```rust
let [x, y] = Point(1, 2)
let [a, b, c] = int[3](10, 20, 30)
```

### Type Aliases

The `alias` keyword can be used to create a new name for an existing type.

```rust
alias Map = hashMap.HashMap<str.Str, int>

func main() : int {
    let m : Map = Map()
    m["Hello"] = 0xe1751aff
    return 0
}
```

## Standard Library

Hoshi-lang's standard library is growing and currently includes:

*   `console`: For console input and output.
*   `file`: An interface for file-like objects.
*   `fs`: For file system operations.
*   `hashMap`: A hash map implementation.
*   `io`: For I/O operations.
*   `json`: For parsing JSON.
*   `math`: For mathematical functions.
*   `runtime`: For runtime-specific functions.
*   `str`: A string library.
*   `threading`: For multi-threaded programming.
*   `vec`: A dynamic array implementation.

## Build and Run

### Build Instructions

**macOS / Linux:**
```shell
make cmake_debug
make build_debug
make package
```

**Windows:**
```shell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Run

```shell
./hoshi_lang [options] <input_file>
```

**Options:**

*   `-o <output_file>`: Specify the output file name.
*   `-D <key> <value>`: Define a macro.

## Documentation

*   [Syntax Definition](/Syntax.bnf)
*   [IR Handbook](/docs/IR.md)
*   [Language Specification](/docs/Spec.md)
*   [Callable Objects & Lambda Expressions](/docs/Callable%20&%20Lambda.md)
*   [Console I/O](/docs/Console.md)
*   [File System](/docs/File%20System.md)
*   [Finalizers](/docs/Finalizers.md)
*   [HashMap](/docs/HashMap.md)
*   [JSON](/docs/JSON.md)
*   [Macros](/docs/Macros.md)
*   [Math](/docs/Math.md)
*   [Nullable and Raw Check Passes](/docs/Nullable%20Check%20&%20Raw%20Check.md)
*   [Result Type](/docs/Result.md)
*   [Runtime](/docs/Runtime.md)
*   [String](/docs/String.md)
*   [Structured Bindings](/docs/Structured%20Bindings.md)
*   [Threading](/docs/Threading.md)
*   [Type Aliases](/docs/Type%20Aliases.md)
*   [Vector](/docs/Vector.md)
*   [Wrapper](/docs/Wrapper.md)
*   [TODO List](/TODO.md)