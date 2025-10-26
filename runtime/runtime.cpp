#include "runtime.h"
#include "runtime/memory/memory.h"

#include <cstdio>

const char ** yoi_argv{};
int yoi_argc{};

int elysia_main(int argc, char *argv[]) {
    yoi_argv = (const char **)argv;
    yoi_argc = argc;
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG) || defined(ELYSIA_RUNTIME_BUILD_PRESERVE_BASIC_INFORMATION)
    printf("[Elysia/DEBUG] hoshi-lang descriptor: %s, build_type: %llu. Runtime linked, invoking yoimiya_entry()...\n", &yoi_desc, yoi_build_type);
    #endif
    YoiIntegerObject *result = yoimiya_entry();
    int resultVal = static_cast<int>(result->value);
    basic_int_gc_refcount_decrease(result);
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG) || defined(ELYSIA_RUNTIME_BUILD_PRESERVE_BASIC_INFORMATION)
    printf("[Elysia/DEBUG] hoshi-lang runtime finished, result: %d.\n", resultVal);
    #endif
#if defined(ELYSIA_RUNTIME_ENABLE_BUILTIN_MEMORY_LEAK_DETECTOR)
    if (runtime_object_allocated > 0) {
        printf("[Elysia/WARNING] hoshi-lang runtime finished with %lld objects allocated, memory leaks detected!\nargv[0]: %s\n", runtime_object_allocated, argv[0]);
#if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
        runtime_debug_print_current_allocated_memory();
#endif
        return -11;
    }
#endif
    return resultVal;
}

YoiObjectArray *runtime_get_argv() {
    auto *argv = (YoiObjectArray *)runtime_object_alloc(sizeof(YoiObjectArray) + yoi_argc * sizeof(char *));
    argv->gc_refcount = 1;
    argv->type_id = 11;
    argv->length = yoi_argc;
    auto **argv_start = (const char **)((char *)&argv->data);
    for (int i = 0; i < yoi_argc; i++) {
        argv_start[i] = yoi_argv[i];
    }
    return argv;
}

void runtime_panic(char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

void *runtime_get_stdin_fp() {
    return stdin;
}

void *runtime_get_stdout_fp() {
    return stdout;
}

void *runtime_get_stderr_fp() {
    return stderr;
}
