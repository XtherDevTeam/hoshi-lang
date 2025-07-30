//
// Created by XIaokang00010 on 2025/7/30.
//

#include <runtime/debug/debug.h>
#include <cstdio>

extern "C" void runtime_debug_print(const char *message) {
    printf("[Elysia/DEBUG] %s\n", message);
}
void runtime_debug_report_current_function(const char *function_name) {
    printf("[Elysia/DEBUG] Entering function %s\n", function_name);
}
