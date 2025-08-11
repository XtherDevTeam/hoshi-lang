# Hoshi Language Specification

## 1. Introduction

Hoshi (also referred to as Yoi-lang in documentation) is a statically-typed, strongly-typed programming language. It aims to support modern programming paradigms, including Object-Oriented Programming (OOP) and Generic Programming (GP), while maintaining a clear and efficient compilation process.

The language compiles to its own high-level intermediate representation (Yoi IR) before being translated to LLVM IR for native code generation. It features a reference-counting-based memory management system for its object model.

## 2. Lexical Structure

### 2.1. Comments

Hoshi supports C-style comments:
- **Single-line comments:** Start with `//` and continue to the end of the line.
- **Multi-line comments:** Start with `/*` and end with `*/`.

### 2.2. Keywords

The following are reserved keywords and cannot be used as identifiers:

`use`, `func`, `interface`, `struct`, `impl`, `let`, `in`, `for`, `forEach`, `while`, `if`, `elif`, `else`, `return`, `continue`, `break`, `cast`, `null`, `constructor`, `import`, `export`, `as`, `from`, `true`, `false`.

### 2.3. Identifiers

Identifiers are used for naming variables, functions, types, etc. They must start with a letter or an underscore (`_`) and can be followed by any number of letters, digits, or underscores.

### 2.4. Literals

- **Integer Literals:** Sequences of digits representing 64-bit signed integers (e.g., `123`, `1048576`).
- **Decimal Literals:** Sequences of digits with a decimal point, representing 64-bit floating-point numbers (e.g., `1919.810`, `3.5`).
- **Boolean Literals:** `true` or `false`.
- **String Literals:** A sequence of characters enclosed in double quotes (`"`). It supports escape sequences like `\n`, `\t`, etc.
- **Character Literals:** A single character enclosed in single quotes (`'`).
- **Null Literal:** `null`, representing the absence of a value.

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

Structs are user-defined composite data types that group variables and methods. They are defined using the `struct` keyword.

```rust
struct Point {
    x: int,
    y: int,
    constructor(x: int, y: int)
}
```

### 3.3. Interfaces

Interfaces define a contract of methods that a `struct` can implement. They are defined with the `interface` keyword and only contain method declarations.

```rust
interface Addable {
    add() : int
}
```

### 3.4. Implementations (`impl`)

The `impl` keyword is used to provide implementations for `struct` methods or to implement an `interface` for a `struct`.

- **Struct method implementation:**
  ```rust
  impl Point {
      constructor(x: int, y: int) {
          this.x = x
          this.y = y
          return this
      }
  }
  ```
- **Interface implementation for a struct:**
  ```rust
  impl Point : Addable {
      add() : int {
          return this.x + this.y
      }
  }
  ```

### 3.5. Arrays

Arrays are fixed-size collections of elements of the same type. The size must be known at compile time.

- **Declaration and Initialization:**
  ```rust
  // An array of 10 integers
  let arr = int[10](1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

  // A 2x3 multi-dimensional array
  let arr2 = int[2][3](1, 2, 3, 4, 5, 6);
  ```
- **Element Access:**
  ```rust
  let first = arr[0];
  let element = arr2[i][j];
  ```

### 3.6. Generic Programming (Templates)

Hoshi supports generic programming through templates, which can be applied to functions and structs. Template parameters are specified using angle brackets `<>`.

```rust
// A generic function
func add_inv<T impl Addable>(a: T) : int {
    let addable = Addable(a)
    return addable.add()
}

// A generic struct
struct TestStruct<T, U> {
    a: T,
    b: U
}
```

## 4. Object Model and Lifecycle

### 4.1. Everything is an Object

All data types in Hoshi are objects allocated on the heap. This includes primitive types like `int` and `bool`, which are wrapped in corresponding object structures (e.g., `YoiIntegerObject`).

### 4.2. Memory Management: Reference Counting

Hoshi uses an automatic reference counting (ARC) system for memory management.
- Every object has a reference count, stored in its header.
- When an object is created or a new reference to it is made (e.g., assignment, passing as an argument), its reference count is incremented.
- When a reference goes out of scope or is reassigned, the object's reference count is decremented.
- When an object's reference count reaches zero, it is deallocated.

This process is handled automatically by the compiler and runtime through `gc_refcount_increase` and `gc_refcount_decrease` functions generated for each type.

### 4.3. Object Creation

- **Structs:** Created by calling their constructor, which is defined in an `impl` block.
  ```rust
  let p1 = Point(1, 2);
  ```
- **Interfaces:** An interface object is created by "casting" a struct instance that implements it. This creates a new interface object containing a pointer to the original struct instance and a virtual method table (v-table).
  ```rust
  let p1 = Point(1, 2);
  let addable = Addable(p1); // Creates an interface object
  ```

## 5. Yoi Intermediate Representation (IR)

The Yoi IR is a high-level, stack-based intermediate representation. It serves as the bridge between the frontend parser and the backend code generator (LLVM).

### 5.1. Key Components

- **`IRModule`**: Represents a single compiled source file, containing tables for functions, structs, interfaces, globals, and string literals.
- **`IRFunctionDefinition`**: Defines a function with its name, arguments, return type, local variable table, and code blocks.
- **`IRCodeBlock`**: A sequence of IR instructions, analogous to a basic block.
- **`IRValueType`**: The IR's own type system, representing Hoshi types as objects (e.g., `integerObject`, `structObject`).
- **`IROperand`**: Arguments for IR instructions, which can be immediate values or indices into various tables.

### 5.2. Execution Model

The IR operates on a temporary value stack for each function. Instructions pop operands from this stack and push results back onto it. All values on the stack are pointers to heap-allocated, reference-counted objects.

### 5.3. Instruction Set Highlights

- **Stack/Memory:** `push_*`, `load_local`, `store_local`, `load_member`, `store_member`.
- **Arithmetic/Logic:** `add`, `sub`, `mul`, `div`, `equal`, `less_than`, etc.
- **Control Flow:** `jump`, `jump_if_true`, `jump_if_false`, `ret`, `ret_none`.
- **Object Lifecycle:** `new_struct`, `new_interface`, `construct_interface_impl`.
- **Function Calls:** `invoke` (static dispatch), `invoke_virtual` (dynamic dispatch via v-table).

## 6. Compilation and Linking

The Hoshi compiler (`hoshi-lang`) performs the following steps:

1.  **Parsing:** The source code (`.hoshi`) is parsed into an Abstract Syntax Tree (AST).
2.  **IR Generation:** The `visitor` traverses the AST to generate Yoi IR, contained within an `IRModule`.
3.  **IR Linking:** The `IRLinker` combines multiple `IRModule`s (if there are imports) into a single, unified `IRModule`. It mangles names to prevent collisions.
4.  **LLVM Codegen:** The `LLVMCodegen` component translates the unified Yoi IR into LLVM IR (`.ll` file). This phase generates the necessary LLVM structures, functions, and GC calls.
5.  **Object Code Generation:** The LLVM IR is compiled into a native object file (`.o` or `.obj`).
6.  **Final Linking:** An external system linker (like `cc` or `cl.exe`) is used to link the object file with the Hoshi runtime library (`libelysia_runtime`) to produce the final executable or library.

## 7. Modules and Foreign Function Interface (FFI)

### 7.1. Modules

Hoshi code can be organized into modules. The `use` keyword imports another Hoshi file, making its public symbols available under a specified alias.

```rust
use test1 "examples/test1.hoshi";
```

### 7.2. Foreign Function Interface (FFI)

Hoshi provides an FFI to interoperate with C-compatible libraries.

- **Importing Functions:** The `import` keyword declares an external function, specifying its signature and the library it comes from. The compiler generates a wrapper to handle the conversion between Hoshi objects and raw C types.
  ```rust
  // Imports puts from the C standard library (via a "builtin" alias)
  import puts(str : string) : int32 from "builtin";
  ```

- **Exporting Functions:** The `export` keyword creates a C-compatible wrapper for a Hoshi function, allowing it to be called from other languages.
  ```rust
  func test(a: int, b: int) : int {
      return a + b
  }

  export test as test; // Exports a C-style function: int test(int a, int b)
  ```

- **Exporting Structs:** Structs can also be exported, which generates a C-compatible layout without the Hoshi object header (i.e., no reference count).
  ```rust
  struct Point { x: int, y: int }
  export Point as Point;
  ```
