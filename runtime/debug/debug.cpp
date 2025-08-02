//
// Created by XIaokang00010 on 2025/7/30.
//

#include <runtime/debug/debug.h>
#include <runtime/build_config.h>
#include <cstdio>

extern "C" void runtime_debug_print(const char *message) {
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] %s\n", message);
    #endif
}
extern "C" void runtime_debug_report_current_function(const char *function_name) {
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Entering function %s\n", function_name);
    #endif
}

extern "C" void runtime_debug_print_address(void *address) {
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Address: %p\n", address);
    #endif
}