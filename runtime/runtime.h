//
// Created by XIaokang00010 on 2025/7/29.
//

#ifndef HOSHI_LANG_RUNTIME_RUNTIME_H
#define HOSHI_LANG_RUNTIME_RUNTIME_H

#include <runtime/memory/memory.h>
#include <runtime/build_config.h>
#include <runtime/debug/debug.h>

extern "C" const char *yoi_desc;

extern "C" const unsigned long long yoi_build_type;

extern "C" YoiIntegerObject* yoimiya_entry();

extern "C" int elysia_main(int argc, char *argv[]);

#endif // HOSHI_LANG_RUNTIME_RUNTIME_H