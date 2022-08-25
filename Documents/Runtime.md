# The runtime environment in hoshi-lang

## Memory management

- **Object Stack**

  For each thread, `hoshi_runtime` will allocate `4MB` memory for the current thread's object index. When a thread exits, `hoshi_runtime` will use `free()` to destroy the thread's object stack and start garbage collection. 

  Additionally, `hoshi_runtime` uses `nullptr` in the object stack to separate function frames. `__hoshi_runtime_ostack_pushf` will be called when the function is called. `__hoshi_runtime_ostack_popf` will be called when the function exits. 

  Each thread has its own stack, and `hoshi_runtime` allocates 32 `__hoshi_runtime_thread` structures for multi-threading. This means that `hoshi_runtime` only allows 32 threads to run concurrently. Each thread's `Object Stack` will be dynamically created when a thread starts.

- **The creation of an object**

  Program should use `__hoshi_runtime_object_new (size : uint)` to create a new object in runtime heap. `__hoshi_runtime_object_new ` will use `malloc()` to make a place for this object. And, save the pointer into the `allocator` and `Object Stack` of invoker's thread. And `__hoshi_runtime_object_new` will return the pointer of the object to invoker. Next, the invoker should invoke the corresponding `new` method of the object.

- **The destruction of an object**

  The destruction of objects will happen during GC (`Garbage Collection`). 

  There's **no** methods like `~ObjectName` in C++ to do something before the object destruct. There's no way for `hoshi_runtime` to know about the methods of an object.

- **GC**

  GC will be started automatically when the `__hoshi_runtime_object_new` be called and the conditions for starting Passive GC are met. There is also a `GC` thread, it will wake up every `2ms` and check the conditions for Active GC. When the conditions are met, GC will also be started automatically.

  When GC started, it will set `blocking` flag to `true`. So, there's no other threads can't create a new object at that time. Then, it will mark every object pointers in `Object Stack` Then, GC will traverse each element of `allocation blocks`, and remove the object which is not marked.

- **Allocator**

  Allocator will save the objects pointer in `allocation blocks`. `allocation blocks` records each object pointer allocated by `__hoshi_runtime_object_new`. Each `allocation block` records pointers to `512` objects. The `allocate blocks` will be saved as a list. When the current `allocation block` is full, `hoshi_runtime` will create a new block. If there's no valid in this block, runtime will mark it as a `empty_block`, and make a counter for it, and if this block still empty after 3 rounds of GC, runtime will destroy this free block.

- **The storage of an object**

  There are 2 areas in an object. The first area is a place to save every object in this object. The second area is used to save some basic types like `int` `uint`, etc. There is a blank of `uint64_t` type in the middle for dividing the two areas.