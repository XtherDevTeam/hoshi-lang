//
// Created by XIaokang00010 on 2025/7/30.
//

#ifndef HOSHI_LANG_RUNTIME_DEBUG_H
#define HOSHI_LANG_RUNTIME_DEBUG_H

#include <cstdint>
extern "C" void runtime_debug_report_current_function(const char *function_name);

extern "C" void runtime_debug_print(const char *message);

extern "C" void runtime_debug_print_address(void *address);

extern "C" void runtime_debug_print_int(int64_t value);

extern "C" void runtime_debug_print_deci(double value);

extern "C" void runtime_debug_print_int_1(int64_t value);

#endif //HOSHI_LANG_RUNTIME_DEBUG_H