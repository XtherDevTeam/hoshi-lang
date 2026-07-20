//
// Created by XIaokang00010 on 2025/7/29.
//

#ifndef HOSHI_LANG_MEMORY_H
#define HOSHI_LANG_MEMORY_H

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <runtime/build_config.h>
#include <runtime/rtti/rtti.h>

class BaconMark {
public:
    enum class Color : unsigned long long { Survive = 0b00, Garbage = 0b01, Attempted = 0b10, Candidate = 0b11 };

private:
    unsigned long long data{0};

    static constexpr unsigned long long COLOR_MASK = 0b011;
    static constexpr unsigned long long BUFFERED_MASK = 0b100;

public:
    Color get_color() const;

    bool is_buffered() const;

    void set_color(Color color);

    void set_buffered(bool buffered);

    bool try_mark_candidate();
};

struct YoiObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    BaconMark bacon_mark;
};

struct YoiObjectArray {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    BaconMark bacon_mark;
    unsigned long long length;
    void *data;
};

struct YoiIntegerObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    BaconMark bacon_mark;
    long long value;

    void acquire();

    void release();
};

struct YoiUnsignedObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    BaconMark bacon_mark;
    unsigned long long value;

    void acquire();

    void release();
};

struct YoiStringObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    BaconMark bacon_mark;
    char *value;

    void acquire();
    void release();
};

struct YoiBooleanObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    BaconMark bacon_mark;
    bool value;

    void acquire();

    void release();
};

struct YoiDecimalObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    BaconMark bacon_mark;
    double value;

    void acquire();

    void release();
};

struct YoiCharObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    BaconMark bacon_mark;
    wchar_t value;

    void acquire();
    void release();
};

struct YoiShortObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    BaconMark bacon_mark;
    int16_t value;

    void acquire();
    void release();
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

void runtime_trace_yoi_object(YoiObject *obj, void (*callback)(YoiObject *obj, void*), void* context);

#ifndef ELYSIA_DISABLE_MEMORY_EXECUTABLE_MAPPING_FEATURE

extern "C" void *runtime_exec_permit_alloc(unsigned long size);

extern "C" void runtime_exec_permit_free(void *ptr);

#endif

#define GC_WRAPPER_IMPL(X, U)                                                                                                                        \
    extern "C" void basic_##X##_gc_refcount_increase(U *obj) { obj->gc_refcount++; }                                                                 \
                                                                                                                                                     \
    extern "C" void basic_##X##_gc_refcount_decrease(U *obj) {                                                                                       \
        obj->gc_refcount--;                                                                                                                          \
        if (obj->gc_refcount <= 0) {                                                                                                                 \
            runtime_finalize_object((YoiObject *)obj);                                                                                               \
        }                                                                                                                                            \
    }

#define GC_WRAPPER_INLINE(X, U)                                                                                                                      \
    static inline void basic_##X##_gc_refcount_increase(U *obj) {                                                                                    \
        if (!obj)                                                                                                                                    \
            return;                                                                                                                                  \
        obj->gc_refcount++;                                                                                                                          \
    }                                                                                                                                                \
                                                                                                                                                     \
    static inline void basic_##X##_gc_refcount_decrease(U *obj) {                                                                                    \
        if (!obj)                                                                                                                                    \
            return;                                                                                                                                  \
        obj->gc_refcount--;                                                                                                                          \
        if (obj->gc_refcount <= 0) {                                                                                                                 \
            runtime_finalize_object((YoiObject *)obj);                                                                                               \
        }                                                                                                                                            \
    }

GC_WRAPPER_INLINE(int, YoiIntegerObject);

GC_WRAPPER_INLINE(decimal, YoiDecimalObject);

GC_WRAPPER_INLINE(bool, YoiBooleanObject);

GC_WRAPPER_INLINE(char, YoiCharObject);

GC_WRAPPER_INLINE(string, YoiStringObject);

GC_WRAPPER_INLINE(unsigned, YoiUnsignedObject);

GC_WRAPPER_INLINE(short, YoiShortObject);

#endif // HOSHI_LANG_MEMORY_H