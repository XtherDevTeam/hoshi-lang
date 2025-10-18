# Threading in Hoshi-lang

Hoshi-lang provides basic support for multi-threaded programming through the `threading` standard library module.

## `Thread` Struct

The `Thread` struct represents a single thread of execution. It is created with a callable object (a function or a lambda) that will be executed in the new thread.

### Methods

-   `constructor(callableObject: func () : none)`: Creates a new `Thread` with the given callable object.
-   `start() : lang.Result<unsigned, int>`: Starts the execution of the thread. Returns a `Result` object containing the thread ID on success, or an error code on failure.
-   `join() : none`: Waits for the thread to complete its execution.
-   `ping() : int`: Checks if the thread is still running.

### Example

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

## `Mutex` Struct

The `Mutex` struct provides a mutual exclusion mechanism to protect shared data from being simultaneously accessed by multiple threads.

### Methods

-   `constructor()`: Creates a new `Mutex`.
-   `lock() : none`: Acquires the mutex lock. If the mutex is already locked by another thread, this call will block until it is released.
-   `unlock() : none`: Releases the mutex lock.
-   `try_lock() : bool`: Attempts to acquire the mutex lock without blocking. Returns `true` if the lock was acquired successfully, and `false` otherwise.

### Example

```rust
use threading "threading"
use runtime "runtime"

func main() : int {
    let mutex = threading.Mutex()
    mutex.lock()
    runtime.puts("Main thread has locked the mutex")
    mutex.unlock()
    return 0
}
```

## `current_tid()`

The `current_tid()` function returns the ID of the currently executing thread.

```rust
use threading "threading"
use runtime "runtime"

func main() : int {
    let tid = threading.current_tid()
    runtime.puts("Current thread ID: " + tid)
    return 0
}
```
