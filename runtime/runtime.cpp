#include "runtime.h"
#include "runtime/memory/memory.h"
#include <cstdio>

int elysia_main(int argc, char *argv[]) {
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG) || defined(ELYSIA_RUNTIME_BUILD_PRESERVE_BASIC_INFORMATION)
    printf("[Elysia/DEBUG] Yoi-lang descriptor: %s, build_type: %llu. Runtime linked, invoking yoimiya_entry()...\n", &yoi_desc, yoi_build_type);
    #endif
    YoiIntegerObject *result = yoimiya_entry();
    int resultVal = static_cast<int>(result->value);
    basic_int_gc_refcount_decrease(result);
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG) || defined(ELYSIA_RUNTIME_BUILD_PRESERVE_BASIC_INFORMATION)
    printf("[Elysia/DEBUG] Yoi-lang runtime finished, result: %d.\n", resultVal);
    #endif
#if defined(ELYSIA_RUNTIME_ENABLE_BUILTIN_MEMORY_LEAK_DETECTOR)
    if (runtime_object_allocated > 0) {
        printf("[Elysia/WARNING] Yoi-lang runtime finished with %lld objects allocated, memory leaks detected!\n", runtime_object_allocated);
#if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
        runtime_debug_print_current_allocated_memory();
#endif
        return -11;
    }
#endif
    return resultVal;
}
