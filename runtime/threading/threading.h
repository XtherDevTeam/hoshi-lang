//
// Created by XIaokang00010 on 2025/9/12.
//

#ifndef HOSHI_LANG_THREADING_H
#define HOSHI_LANG_THREADING_H

#include "runtime/memory/memory.h"
#include <pthread.h>
#include <signal.h>

struct YoiVoidCallableInterface {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    void *this_ptr;
    void *(*gc_inc_func)(void *this_ptr);
    void *(*gc_dec_func)(void *this_ptr);
    void *(*callable)(void *this_ptr);
};

struct YoiResultUnsignedAndIntObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    YoiUnsignedObject *ok;
    YoiIntegerObject *err;
};

extern "C" YoiResultUnsignedAndIntObject *runtime_start_thread(YoiVoidCallableInterface *callable);

extern "C" YoiIntegerObject *runtime_thread_join(YoiUnsignedObject *thread_id);

extern "C" YoiUnsignedObject *runtime_get_thread_id();

extern "C" YoiIntegerObject *runtime_ping_thread(YoiUnsignedObject *thread_id);

#endif //HOSHI_LANG_THREADING_H