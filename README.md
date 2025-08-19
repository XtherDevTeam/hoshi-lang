# The Hoshi-lang Programming Language

## Intro

> This is one of my practice during Senior High School period. `hoshi` means both $\mathop{desire}\limits^{欲しい}$ and $\mathop{stars}\limits^{星}$ in Japanese, which also represents my silly wishes: I hope some day I would become the stars I once live up to.

Hoshi-lang is a statically-typed, strong-typed programming language that supports modern programming design patterns like Generic Programming and Object-Oriented Programming.

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

impl Greeter for Person {
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

Here is an example.

## Object-Oriented Programming

Hoshi-lang's OOP is based on a composition model using three core components: `interface`, `struct`, and `impl`.

-   `interface`: Defines an abstract contract with a set of method signatures.
-   `struct`: A concrete data structure that groups fields and methods.
-   `impl`: Implements the methods for a `struct` or an `interface` for a `struct`.

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

## New Features

### Dynamic Arrays

Hoshi-lang now supports dynamic arrays, which can grow or shrink at runtime.

-   **Creation:**
    ```rust
    // Create a dynamic array with initial elements
    let dyn_arr = int[](1, 2, 3)

    // Create a dynamic array with a specific size
    let dyn_arr2 = string[](10) // An array of 10 null strings
    ```
-   **Length Property:** The `.length` property can be used to get the current number of elements in any array (fixed or dynamic).
    ```rust
    let len = dyn_arr.length // len will be 3
    ```

### Type Introspection

You can inspect an object's type at runtime using the `typeid` and `interfaceof` operators.

-   **`typeid`**: Returns a unique integer ID for a type.
    ```rust
    let id = typeid(int)
    let p = Person("test")
    let p_id = typeid(p)
    ```
-   **`interfaceof`**: Checks if an object implements a specific interface.
    ```rust
    if (p interfaceof Greeter) {
      io.println("This object is a Greeter!")
    }
    ```

### Dynamic Casting

Safely cast an interface object back to its concrete `struct` type using `dyn_cast`. If the cast fails, it returns `null`.

```rust
let g = Greeter(Person("test"))

// Attempt to cast the Greeter back to a Person
let p = dyn_cast(g, Person)

if (p != null) {
  io.println("Cast successful: " + p.name)
} else {
  io.println("Cast failed.")
}
```

## Modules

Hoshi-lang supports modular programming using the `use` statement to import other source files.

```rust
// Import symbols from "std/io.hoshi" under the alias "io"
use io "std/io"
```

## Generic Programming

Hoshi-lang supports Generic Programming via `template`s, which can be applied to both functions and structs.

```rust
struct Container<T> {
  item: T,
}

func print_item<T>(c: Container<T>) {
  io.println(c.item)
}
```

## Variadic Arguments

Variadic arguments are supported in template functions via the `...` keyword, allowing a function to accept a variable number of arguments, which would be converted to `lang.NullInterface[]`

```rust
func my_printf(format: string, args: ...) {
  // ... implementation ...
}
```

## Build and Run

### Build Instructions

**macOS / Linux:**
```shell
mkdir build
cd build
cmake ..
make
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
./hoshi_lang [filename] -o [output filename]
```

## Further Reading

-   [Syntax Definition](/Syntax.bnf)
-   [IR Handbook](/docs/IR.md)
-   [Language Specification](/docs/Spec.md)
-   [TODO List](/TODO.md)
