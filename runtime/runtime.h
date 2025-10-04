//
// Created by XIaokang00010 on 2025/7/29.
//

#ifndef HOSHI_LANG_RUNTIME_RUNTIME_H
#define HOSHI_LANG_RUNTIME_RUNTIME_H

#include <cstdio>
#include <runtime/memory/memory.h>
#include <runtime/build_config.h>
#include <runtime/debug/debug.h>
#include <runtime/rtti/rtti.h>
#include <runtime/threading/threading.h>

extern "C" const char ** yoi_argv;

extern "C" int yoi_argc;

extern "C" char yoi_desc;

extern "C" const unsigned long long yoi_build_type;

extern "C" YoiIntegerObject* yoimiya_entry();

extern "C" YoiObjectArray *runtime_get_argv();

extern "C" void runtime_panic(char *message);

extern "C" int elysia_main(int argc, char *argv[]);

extern "C" void *runtime_get_stdin_fp();

extern "C" void *runtime_get_stdout_fp();

extern "C" void *runtime_get_stderr_fp();

#endif // HOSHI_LANG_RUNTIME_RUNTIME_H