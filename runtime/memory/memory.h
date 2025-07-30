//
// Created by XIaokang00010 on 2025/6/29.
//

#ifndef HOSHI_LANG_MEMORY_H
#define HOSHI_LANG_MEMORY_H

struct YoiIntegerObject {
    unsigned long long gc_refcount;
    long long value;
};

struct YoiStringObject {
    unsigned long long gc_refcount;
    char *value;
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
    char value;
};

extern "C" void *runtime_object_alloc(long size_in_bytes);

extern "C" void runtime_finalize_object(void *object);

extern "C" void basic_int_gc_refcount_increase(YoiIntegerObject *obj);

extern "C" void basic_int_gc_refcount_decrease(YoiIntegerObject *obj);

extern "C" void basic_decimal_gc_refcount_increase(YoiIntegerObject *obj);

extern "C" void basic_decimal_gc_refcount_decrease(YoiIntegerObject *obj);

extern "C" void basic_bool_gc_refcount_increase(YoiIntegerObject *obj);

extern "C" void basic_bool_gc_refcount_decrease(YoiIntegerObject *obj);

extern "C" void basic_char_gc_refcount_increase(YoiIntegerObject *obj);

extern "C" void basic_char_gc_refcount_decrease(YoiIntegerObject *obj);

extern "C" void basic_string_gc_refcount_increase(YoiStringObject *obj);

extern "C" void basic_string_gc_refcount_decrease(YoiStringObject *obj);

#endif //HOSHI_LANG_MEMORY_H