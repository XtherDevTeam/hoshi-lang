//
// Created by XIaokang00010 on 2025/7/29.
//

#ifndef HOSHI_LANG_MEMORY_H
#define HOSHI_LANG_MEMORY_H

#include <cstdint>
#include <cstdio>
#include <runtime/build_config.h>

struct YoiObject {
    unsigned long long gc_refcount;
};

struct YoiIntegerObject {
    unsigned long long gc_refcount;
    long long value;
};

struct YoiStringObject {
    unsigned long long gc_refcount;
    wchar_t *value;
};

struct YoiBooleanObject {
    unsigned long long gc_refcount;
    bool value;
};

struct YoiDecimalObject {
    unsigned long long gc_refcount;
    double value;
};

struct YoiCharObject {
    unsigned long long gc_refcount;
    wchar_t value;
};

struct AllocatedMemoryList {
    AllocatedMemoryList *prev;
    AllocatedMemoryList *next;
    void *memory;
    unsigned long size;
};

#ifdef ELYSIA_RUNTIME_BUILD_TYPE_DEBUG
extern "C" AllocatedMemoryList *allocated_memory_list;

extern "C" void runtime_debug_print_current_allocated_memory();
#endif

extern "C" int64_t runtime_object_allocated;

extern "C" void *runtime_object_alloc(unsigned long size_in_bytes);

extern "C" void runtime_finalize_object(void *object);

#define GC_WRAPPER_DECL(X, U) extern "C" void basic_##X##_gc_refcount_increase(U* obj);      \
                                                                                            \
extern "C" void basic_##X##_gc_refcount_decrease(U* obj);                                   \

#define GC_WRAPPER_IMPL(X, U) extern "C" void basic_##X##_gc_refcount_increase(U* obj) {     \
    obj->gc_refcount++;                                                                     \
}                                                                                           \
                                                                                            \
extern "C" void basic_##X##_gc_refcount_decrease(U* obj) {                                  \
    obj->gc_refcount--;                                                                     \
    if (obj->gc_refcount <= 0) {                                                            \
        runtime_finalize_object((void*)obj);                                                 \
    }                                                                                       \
}


GC_WRAPPER_DECL(int, YoiIntegerObject);

GC_WRAPPER_DECL(decimal, YoiDecimalObject);

GC_WRAPPER_DECL(bool, YoiBooleanObject);

GC_WRAPPER_DECL(char, YoiCharObject);

GC_WRAPPER_DECL(string, YoiStringObject);

#endif //HOSHI_LANG_MEMORY_H