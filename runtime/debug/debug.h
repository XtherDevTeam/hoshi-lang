//
// Created by XIaokang00010 on 2025/7/30.
//

#ifndef HOSHI_LANG_RUNTIME_DEBUG_H
#define HOSHI_LANG_RUNTIME_DEBUG_H

extern "C" void runtime_debug_report_current_function(const char *function_name);

extern "C" void runtime_debug_print(const char *message);

#endif //HOSHI_LANG_RUNTIME_DEBUG_H