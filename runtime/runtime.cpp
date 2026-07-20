#include "runtime.h"
#include "runtime/bacon/bacon.h"
#include "runtime/hperf/hperf.h"
#include "runtime/memory/memory.h"

// mimalloc requirement
#ifdef _WIN32
#pragma comment(lib, "Advapi32.lib")
#endif

#include <cstdio>
#include <cstring>

const char ** yoi_argv{};
int yoi_argc{};

int elysia_main(int argc, char *argv[]) {
    yoi_argv = (const char **)argv;
    yoi_argc = argc;
    #if defined(ELYSIA_RUNTIME_HPERF_ENABLE)
    char *enable_hperf = nullptr;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--perf") == 0) {
            printf("[ELysia/INFO] hoshi-lang runtime: hperf enabled.\n");
            if (i + 1 >= argc) {
                printf("[Elysia/ERROR] hoshi-lang runtime: --perf option requires an argument.\n");
                return -1;
            }
            enable_hperf = argv[i + 1];
        }
    }
    if (enable_hperf) 
        hperf_init(enable_hperf);
    #endif
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG) || defined(ELYSIA_RUNTIME_BUILD_PRESERVE_BASIC_INFORMATION)
    printf("[Elysia/DEBUG] hoshi-lang descriptor: %s, build_type: %llu. Runtime linked, invoking yoimiya_entry()...\n", &yoi_desc, yoi_build_type);
    #endif
    bacon_init();
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
    #if defined(ELYSIA_RUNTIME_HPERF_ENABLE)
    if (enable_hperf) {
        hperf_write_report(hperf_report_filename);
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
