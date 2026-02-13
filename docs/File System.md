# File System in Hoshi-lang

Hoshi-lang provides a set of standard library modules for interacting with the file system.

## `file` Module

The `file` module defines the `AbstractFile` interface, which provides a common set of methods for interacting with file-like objects.

### `AbstractFile` Interface

-   `read(size: int): char[]`: Reads a specified number of bytes from the file.
-   `write(data: char[]) : int`: Writes a byte array to the file.
-   `close() : none`: Closes the file.
-   `seek(offset: int, whence: int) : int`: Changes the file position.
-   `tell() : int`: Returns the current file position.
-   `flush() : none`: Flushes the file's buffer.
-   `is_eof() : bool`: Returns `true` if the end of the file has been reached.
-   `size() : int`: Returns the size of the file.
-   `is_seekable() : bool`: Returns `true` if the file supports random access.
-   `is_readable() : bool`: Returns `true` if the file is readable.
-   `is_writable() : bool`: Returns `true` if the file is writable.
-   `is_closed() : bool`: Returns `true` if the file is closed.
-   `getch() : char`: Reads a single character from the file.

## `fs` Module

The `fs` module provides functions for interacting with the file system.

### Functions

-   `open(path: str.Str, mode: str.Str) : lang.Result<OSFile, str.Str>`: Opens a file and returns a `Result` containing an `OSFile` object on success, or an error string on failure.
-   `stdin() : OSFile`: Returns a handle to the standard input stream.
-   `stdout() : OSFile`: Returns a handle to the standard output stream.
-   `stderr() : OSFile`: Returns a handle to the standard error stream.

### `OSFile` Struct

The `OSFile` struct is a concrete implementation of the `AbstractFile` interface for operating system files.

## `io` Module

The `io` module provides an `ArrayBuffer` struct that implements the `AbstractFile` interface for in-memory I/O.

### `ArrayBuffer` Struct

The `ArrayBuffer` struct allows you to treat a byte array as a file.

### Example

```hoshi
use fs "fs"
use file "file"
use str "str"

func main() : int {
    let path = str.Str("test.txt")
    let content = str.Str("Hello, world!")
    let f = fs.open(path, str.Str("w+")).unwrap() as file.AbstractFile
    f.write(content.data)
    f.close()
    return 0
}
```
