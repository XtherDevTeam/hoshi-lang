//
// Created by XIaokang00010 on 2025/7/31.
//

#ifndef HOSHI_LANG_STRING_H
#define HOSHI_LANG_STRING_H

#include <cstdint>
#include <cstring>
#include <runtime/memory/memory.h>
#include <string_view>

extern "C" YoiIntegerObject *basic_string_length(YoiStringObject *obj);

extern "C" YoiStringObject *basic_string_add(YoiStringObject *obj1, YoiStringObject *obj2);

extern "C" YoiCharObject *basic_string_at(YoiStringObject *obj, YoiIntegerObject *index);

extern "C" YoiStringObject * basic_string_slice(YoiStringObject *obj, YoiIntegerObject *start, YoiIntegerObject *end);

#endif //HOSHI_LANG_STRING_H