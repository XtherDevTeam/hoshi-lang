# Hoshi-lang Language Specification

This document provides a specification of the Hoshi-lang programming language.

## 1. Lexical Structure

### 1.1. Identifiers

Identifiers start with a letter or underscore, followed by any number of letters, numbers, or underscores.

### 1.2. Keywords

The following are reserved keywords and cannot be used as identifiers:

`use`, `interface`, `struct`, `impl`, `func`, `let`, `if`, `else`, `for`, `while`, `return`, `true`, `false`, `null`, `typeid`, `interfaceof`, `dyn_cast`, `template`, `operator`, `alias`, `finalizer`, `callable`

### 1.3. Literals

-   **Integer Literals:** e.g., `123`, `-456`
-   **Unsigned Integer Literals:** e.g., `123u`
-   **Short Integer Literals:** e.g., `123s`
-   **Decimal Literals:** e.g., `3.14`, `-0.01`
-   **Boolean Literals:** `true`, `false`
-   **String Literals:** e.g., `"hello"`
-   **Character Literals:** e.g., `'a'`
-   **Null Literal:** `null`

## 2. Types

Hoshi-lang is a statically-typed language. The following are the built-in types:

-   `int`: A 64-bit signed integer.
-   `unsigned`: A 64-bit unsigned integer.
-   `short`: A 16-bit signed integer.
-   `deci`: A 64-bit floating-point number.
-   `bool`: A boolean value (`true` or `false`).
-   `string`: A string of characters.
-   `char`: A single character.
-   `ptr`: A raw pointer.

## 3. Structs

Structs are user-defined data types that can contain fields and methods.

```rust
struct Point {
    x: int,
    y: int,
    constructor(x: int, y: int)
}
```

## 4. Interfaces

Interfaces define a contract of methods that a `struct` can implement.

```rust
interface Greeter {
    say() : none
}
```

## 5. Functions

Functions are defined with the `func` keyword.

```rust
func add(a: int, b: int) : int {
    return a + b
}
```

## 6. Operator Overloading

See [Operator Overloading](/docs/Operator%20Overloading.md).

## 7. Interface Templates

See [Generic Programming with Templates](/docs/Template.md).

## 8. Standard Library

See [Standard Library](/docs/Standard%20Library.md).
