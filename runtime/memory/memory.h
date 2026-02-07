//
// Created by XIaokang00010 on 2025/7/29.
//

#ifndef HOSHI_LANG_MEMORY_H
#define HOSHI_LANG_MEMORY_H

#include <cstdint>
#include <cstdlib>
#include <runtime/build_config.h>
#include <runtime/rtti/rtti.h>

struct YoiObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
};

struct YoiObjectArray {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    unsigned long long length;
    void *data;
};

struct YoiIntegerObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    long long value;
};

struct YoiUnsignedObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    unsigned long long value;
};

struct YoiStringObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    char *value;
};

struct YoiBooleanObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    bool value;
};

struct YoiDecimalObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    double value;
};

struct YoiCharObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
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

#if defined(ELYSIA_RUNTIME_ENABLE_BUILTIN_MEMORY_LEAK_DETECTOR)
extern "C" int64_t runtime_object_allocated;
#endif

extern "C" void *runtime_object_alloc_report(size_t size, void *object);

extern "C" void runtime_finalize_object_report(YoiObject *object);

extern "C" void runtime_finalize_object(YoiObject *object);

extern "C" void *runtime_object_alloc(unsigned long size);

extern "C" YoiIntegerObject *runtime_get_string_array_data_pointer(YoiObjectArray *array);

#ifndef ELYSIA_DISABLE_MEMORY_EXECUTABLE_MAPPING_FEATURE

extern "C" void *runtime_exec_permit_alloc(unsigned long size);

extern "C" void runtime_exec_permit_free(void *ptr);

#endif

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
        runtime_finalize_object((YoiObject*)obj);                                            \
    }                                                                                       \
}


GC_WRAPPER_DECL(int, YoiIntegerObject);

GC_WRAPPER_DECL(decimal, YoiDecimalObject);

GC_WRAPPER_DECL(bool, YoiBooleanObject);

GC_WRAPPER_DECL(char, YoiCharObject);

GC_WRAPPER_DECL(string, YoiStringObject);

#endif //HOSHI_LANG_MEMORY_H