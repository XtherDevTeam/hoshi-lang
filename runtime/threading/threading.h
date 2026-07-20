//
// Created by XIaokang00010 on 2025/9/12.
//

#ifndef HOSHI_LANG_THREADING_H
#define HOSHI_LANG_THREADING_H

#include "runtime/memory/memory.h"
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
typedef HANDLE YoiThreadHandle;
typedef DWORD YoiThreadId;
#else
#include <pthread.h>
#include <signal.h>
typedef pthread_t YoiThreadHandle;
typedef pthread_t YoiThreadId;
#endif

struct YoiVoidCallableInterface {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    unsigned long long bacon_mark;
    void *this_ptr;
    void *(*gc_inc_func)(void *this_ptr);
    void *(*gc_dec_func)(void *this_ptr);
    void *(*callable)(void *this_ptr);
};

struct YoiResultUnsignedAndIntObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    unsigned long long bacon_mark;
    YoiUnsignedObject *ok;
    YoiIntegerObject *err;
};

extern "C" YoiResultUnsignedAndIntObject *runtime_start_thread(YoiVoidCallableInterface *callable);

extern "C" YoiIntegerObject *runtime_thread_join(YoiUnsignedObject *thread_handle);

extern "C" uint64_t runtime_get_thread_id();

extern "C" uint64_t runtime_thread_hardware_concurrency();

extern "C" YoiIntegerObject *runtime_ping_thread(YoiUnsignedObject *thread_id_obj);

extern "C" YoiResultUnsignedAndIntObject *runtime_thread_new_mutex_lock();

extern "C" void runtime_thread_finalize_mutex_lock(YoiUnsignedObject *handle);

extern "C" void runtime_thread_mutex_lock(YoiUnsignedObject *mutex_handle);

extern "C" void runtime_thread_mutex_unlock(YoiUnsignedObject *mutex_handle);

extern "C" YoiIntegerObject *runtime_thread_mutex_try_lock(YoiUnsignedObject *mutex_handle);

extern "C" YoiResultUnsignedAndIntObject *runtime_thread_new_condition();

extern "C" void runtime_thread_condition_wait(YoiUnsignedObject *condition_handle, YoiUnsignedObject *mutex_handle);

extern "C" void runtime_thread_condition_signal(YoiUnsignedObject *condition_handle);

extern "C" void runtime_thread_finalize_condition(YoiUnsignedObject *handle);

#endif //HOSHI_LANG_THREADING_H