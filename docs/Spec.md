# Hoshi Language Specification

## 1. Introduction

Hoshi is a statically-typed, strongly-typed programming language. It aims to support modern programming paradigms, including Object-Oriented Programming (OOP) and Generic Programming (GP), while maintaining a clear and efficient compilation process.

The language compiles to its own high-level intermediate representation (IR) before being translated to LLVM IR for native code generation. It features a reference-counting-based memory management system for its object model.

## 2. Lexical Structure

### 2.1. Comments

- **Single-line comments:** `// ...`
- **Multi-line comments:** `/* ... */`

### 2.2. Keywords

`use`, `func`, `interface`, `struct`, `impl`, `let`, `in`, `for`, `while`, `if`, `elif`, `else`, `return`, `continue`, `break`, `cast`, `dyn_cast`, `typeid`, `interfaceof`, `null`, `constructor`, `import`, `export`, `as`, `from`, `true`, `false`.

## 3. Types and Data Structures

In Hoshi, all data types, including primitives, are treated as objects.

### 3.1. Primitive Types

- **`int`**: A 64-bit signed integer.
- **`deci`**: A 64-bit floating-point number.
- **`bool`**: A boolean value (`true` or `false`).
- **`char`**: A single character.
- **`string`**: A sequence of characters.
- **`none`**: A type representing no value, similar to `void`.

### 3.2. Structs

Structs are user-defined composite data types that group variables and methods.

```rust
struct Point {
    x: int,
    y: int,
    constructor(x: int, y: int)
}
```

### 3.3. Interfaces

Interfaces define a contract of methods that a `struct` can implement.

```rust
interface Addable {
    add() : int
}
```

### 3.4. Implementations (`impl`)

The `impl` keyword is used to provide implementations for `struct` methods or to implement an `interface` for a `struct`.

### 3.5. Arrays and Dynamic Arrays

#### Fixed-Size Arrays

Arrays are fixed-size collections of elements of the same type. The size must be known at compile time.

- **Declaration and Initialization:**
  ```rust
  // An array of 10 integers
  let arr = int[10](1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
  ```

#### Dynamic Arrays

Dynamic arrays can grow or shrink at runtime.

- **Declaration and Initialization:**
  ```rust
  // A dynamic array with 3 initial elements
  let dyn_arr = int[](1, 2, 3)

  // A dynamic array of 10 uninitialized integers
  let dyn_arr2 = int[](10)
  ```

#### The `.length` Property

All array types have a read-only `.length` property.

```rust
let len = dyn_arr.length // Returns 3
```

### 3.6. Generic Programming (Templates)

Hoshi supports generic programming through templates, which can be applied to functions and structs.

```rust
struct TestStruct<T, U> {
    a: T,
    b: U
}
```

## 4. Object Model and Lifecycle

### 4.1. Everything is an Object

All data types in Hoshi are objects allocated on the heap. This includes primitive types like `int` and `bool`, which are wrapped in corresponding object structures.

### 4.2. Memory Management: Reference Counting

Hoshi uses an automatic reference counting (ARC) system. The compiler and runtime automatically manage object lifecycles.

### 4.3. Object Creation

- **Structs:** Created by calling their constructor.
- **Interfaces:** An interface object is created by "casting" a struct instance that implements it. This creates a new interface object containing a pointer to the original struct instance and a virtual method table (v-table).

## 5. Type Introspection and Casting

### 5.1. `typeid` Operator

The `typeid` operator returns a unique `int` identifier for any type or value at runtime.

```rust
let int_id = typeid(int)

let p = Point(1, 2)
let point_id = typeid(p)

if (typeid(p) == typeid<Point>) { ... }
```

### 5.2. `interfaceof` Operator

The `interfaceof` operator checks if an object instance implements a given interface. It returns `true` or `false`.

```rust
let p = Point(1, 2)
if (p interfaceof Addable) {
    // This will be true
}
```

### 5.3. Dynamic Casting (`dyn_cast`)

The `dyn_cast` operator provides a safe way to cast an interface object back to its original, concrete `struct` type. If the cast is successful, it returns the `struct` object otherwise, it returns `null`.

```rust
func process(a: Addable) {
    let p = dyn_cast<Point>(a)
    if (p != null) {
        // Cast successful, we can now access Point's fields
        io.println("Point x: " + p.x)
    } else {
        io.println("The provided Addable was not a Point.")
    }
}
```

## 6. Compilation and Linking

The Hoshi compiler (`hoshi-lang`) performs the following steps:

1.  **Parsing:** Source code (`.hoshi`) is parsed into an Abstract Syntax Tree (AST).
2.  **IR Generation:** A `visitor` traverses the AST to generate Hoshi IR.
3.  **LLVM Codegen:** The Hoshi IR is translated into LLVM IR (`.ll`).
4.  **Object Code Generation:** LLVM IR is compiled into a native object file (`.o`).
5.  **Final Linking:** An external linker (`cc` or `cl.exe`) links the object file with the Hoshi runtime library (`libelysia_runtime`) to produce the final executable.

## 7. Modules and Foreign Function Interface (FFI)

### 7.1. Modules

The `use` keyword imports another Hoshi file.

```rust
use test1 "examples/test1.hoshi"
```

### 7.2. Foreign Function Interface (FFI)

Hoshi provides an FFI to interoperate with C-compatible libraries.

- **Importing:** The `import` keyword declares an external function.
- **Exporting:** The `export` keyword creates a C-compatible wrapper for a Hoshi function or `struct`.
