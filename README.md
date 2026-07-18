# The hoshi-lang Programming Language

## Intro

> This is one of my practice during Senior High School period. `hoshi` means both $\mathop{desire}\limits^{欲しい}$ and $\mathop{stars}\limits^{星}$, which also represents my silly wishes: I hope some day I would become the stars I once live up to.

hoshi-lang is a statically-typed, general-purpose programming language with a focus on performance, safety, and modern language features. It is designed to be a simple yet powerful tool for building a wide range of applications.

This project is currently under active development and is a personal exploration into language design and implementation.

[![Checkout the Wiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/XtherDevTeam/hoshi-lang)

## Features

*   **Object-Oriented:** hoshi-lang's OOP is based on a composition model using `interface`, `struct`, and `impl`.
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

```hoshi
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

In hoshi-lang, **all data types are heap-allocated objects**. This includes primitives like `int`, `bool`, etc., which are "boxed" into object wrappers. The compiler and runtime manage these objects through pointers.

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

hoshi-lang now supports callable objects and lambda expressions, allowing for more flexible and functional programming styles.

```hoshi
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

hoshi-lang now has basic support for multi-threading.

```hoshi
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

```hoshi
let [x, y] = Point(1, 2)
let [a, b, c] = int[3](10, 20, 30)
```

### Type Aliases

The `alias` keyword can be used to create a new name for an existing type.

```hoshi
alias Map = hashMap.HashMap<str.Str, int>

func main() : int {
    let m : Map = Map()
    m["Hello"] = 0xe1751aff
    return 0
}
```

## Standard Library

hoshi-lang's standard library is growing and currently includes:

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

## Benchmarks

Below are benchmark results comparing hoshi-lang with other popular languages across various tasks. All benchmarks were performed on a macOS system with an Apple M1 chip.

### Performance Comparisons

The following table shows execution times for several computational benchmarks. Lower values are better.

| Benchmark | hoshi-lang | C++ (Clang++) | Python 3 | Java (OpenJDK) |
| :--- | :--- | :--- | :--- | :--- |
| **Fib(40)** | **0.34s** | 0.58s | 10.23s | - |
| **Basic Type Loop** | **0.35s** | 0.54s | 9.53s | - |
| **JSON Parsing** | **0.34s** | - | - | - |
| **String Ops** | **0.18s** | 0.39s (O3) | 0.19s | 1.15s |

### Data Structures: Stack vs. Heap

With the introduction of `datastruct`, hoshi-lang now supports stack-allocated value types, significantly improving performance for small, short-lived data structures by eliminating heap allocation and ARC overhead.

| Language | Implementation | Result (10M iterations) |
| :--- | :--- | :--- |
| **hoshi-lang** | **Datastruct (Stack)** | **< 1ms** |
| hoshi-lang | Legacy Struct (Heap) | 237ms |
| C++ | Value Type (Stack) | 24ms |
| Java | Heap Object | 6ms |
| Python | Legacy Class | 759ms |

### Binary Size

The following sizes were measured for minimal programs in release mode.

| Program | Binary Size (Release) |
| :--- | :--- |
| `size1.hoshi` (Minimal) | ~39 KB |
| `size2.hoshi` (Basic Logic) | ~39 KB |
| `size3.hoshi` (Standard Lib) | ~58 KB |

## Documentation

*   [Syntax Definition](Syntax.bnf)
*   [Language Specification](docs/Spec.md)
*   [IR Handbook](docs/IR.md)
*   [Data Structures (Value Types)](docs/Datastruct.md)
*   [Arithmetic and Array](docs/Array.md)
*   [Callable Objects & Lambda Expressions](docs/Callable%20&%20Lambda.md)
*   [Console I/O](docs/Console.md)
*   [Direct Assignment](docs/Direct%20Assignment.md)
*   [File System](docs/File%20System.md)
*   [Finalizers](docs/Finalizers.md)
*   [HashMap](docs/HashMap.md)
*   [Interface and Implementation](docs/Interface.md)
*   [JSON Parsing](docs/JSON.md)
*   [Macros](docs/Macros.md)
*   [Mathematical Functions](docs/Math.md)
*   [Null Values and Safety](docs/Null.md)
*   [Nullable and Raw Check Passes](docs/Nullable%20Check%20&%20Raw%20Check.md)
*   [Operator Overloading](docs/Operator%20Overloading.md)
*   [Result Type](docs/Result.md)
*   [Runtime Functions](docs/Runtime.md)
*   [String Library](docs/String.md)
*   [Structured Bindings](docs/Structured%20Bindings.md)
*   [Templates and Generics](docs/Template.md)
*   [Optimization Strategy](docs/The%20Optimization%20Strategy%20of%20Interface%20Allocation%20and%20Virtual%20Invocation%20Reduction.md)
*   [Threading](docs/Threading.md)
*   [Type Aliases](docs/Type%20Aliases.md)
*   [Vector Implementation](docs/Vector.md)
*   [Wrapper Objects](docs/Wrapper.md)
*   [TODO List](TODO.md)