# Runtime in Hoshi-lang

The `runtime` standard library module provides low-level functions for interacting with the Hoshi-lang runtime environment.

**Note:** The functions in this module are generally unsafe and should be used with caution.

## Functions

-   `runtime_get_string_array_data_pointer(arr: char[]) : int`: Returns a raw pointer to the underlying data of a character array.
-   `strlen(str: ptr) : unsigned`: Returns the length of a null-terminated C-style string.
-   `memcmp(a: ptr, b: ptr, n: int) : int32`: Compares two blocks of memory.
-   `memcpy(dest: ptr, src: ptr, n: int) : ptr`: Copies a block of memory.
-   `puts(str: ptr) : int32`: Prints a null-terminated C-style string to the console.
-   `runtime_debug_print_int_1(value: int) : none`: Prints an integer to the console for debugging purposes.
-   `fopen(filename: ptr, mode: ptr) : ptr`: Opens a file.
-   `fclose(stream: ptr) : int32`: Closes a file.
-   `fread(ptr: ptr, size: int, nmemb: int, stream: ptr) : unsigned`: Reads from a file.
-   `fseek(stream: ptr, offset: int, whence: int) : unsigned`: Seeks to a position in a file.
-   `ftell(stream: ptr) : int32`: Returns the current position in a file.
-   `fflush(stream: ptr) : int32`: Flushes a file's buffer.
-   `fwrite(ptr: ptr, size: int, nmemb: int, stream: ptr) : unsigned`: Writes to a file.
-   `feof(stream: ptr) : int32`: Checks for the end of a file.
-   `fgetc(stream: ptr) : int32`: Reads a character from a file.
-   `runtime_start_thread(f: func () : none) : lang.Result<unsigned, int>`: Starts a new thread.
-   `runtime_thread_join(thread_id: unsigned) : int`: Waits for a thread to finish.
-   `runtime_get_thread_id() : unsigned`: Returns the current thread's ID.
-   `runtime_ping_thread(thread_id: unsigned) : int`: Checks if a thread is still running.
-   `runtime_thread_new_mutex_lock() : lang.Result<unsigned, int>`: Creates a new mutex.
-   `runtime_thread_mutex_lock(mutex_id: unsigned) : none`: Locks a mutex.
-   `runtime_thread_mutex_unlock(mutex_id: unsigned) : none`: Unlocks a mutex.
-   `runtime_thread_finalize_mutex_lock(mutex_id: unsigned) : none`: Destroys a mutex.
-   `runtime_thread_mutex_try_lock(mutex_id: unsigned) : int`: Tries to lock a mutex.
-   `runtime_get_stdin_fp() : ptr`: Returns a raw pointer to the standard input stream.
-   `runtime_get_stdout_fp() : ptr`: Returns a raw pointer to the standard output stream.
-   `runtime_get_stderr_fp() : ptr`: Returns a raw pointer to the standard error stream.
