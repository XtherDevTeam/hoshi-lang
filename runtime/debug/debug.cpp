//
// Created by XIaokang00010 on 2025/7/30.
//

#include <cstdint>
#include <runtime/debug/debug.h>
#include <runtime/build_config.h>
#include <cstdio>

#if defined (ELYSIA_RUNTIME_HPERF_ENABLE)
#include <runtime/hperf/hperf.h>
#endif

extern "C" void runtime_debug_print(const char *message) {
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] %s\n", message);
    #endif
}
extern "C" void runtime_debug_report_current_function(const char *function_name) {
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Entering function %s\n", function_name);
    #endif
    hperf_report_func_enter(function_name);
}

extern "C" void runtime_debug_print_address(void *address) {
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Address: %p\n", address);
    #endif
}

void runtime_debug_print_int(int64_t value) {
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Debug Integer: %lld\n", value);
    #endif
}

void runtime_debug_print_int_1(int64_t value) {
    printf("[Elysia/DEBUG] Debug Integer: %lld\n", value);
}

void runtime_debug_print_deci(double value) {
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Debug Double: %lf\n", value);
    #endif
}

void runtime_debug_report_leave_function(const char *function_name) {
    #if defined (ELYSIA_RUNTIME_HPERF_ENABLE)
    hperf_report_func_leave(function_name);
    #endif
}
