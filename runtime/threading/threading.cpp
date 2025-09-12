//
// Created by XIaokang00010 on 2025/9/12.
//

#include "runtime/memory/memory.h"
#include <runtime/threading/threading.h>

YoiResultUnsignedAndIntObject *runtime_start_thread(YoiVoidCallableInterface *callable) {
    pthread_t thread_id;
    auto result = pthread_create(&thread_id, nullptr, callable->callable, callable->this_ptr);
    auto yoi_result = (YoiResultUnsignedAndIntObject *)runtime_object_alloc(sizeof(YoiResultUnsignedAndIntObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 16; // leave it be for now

    if (result != 0) {
        yoi_result->ok = nullptr;
        yoi_result->err = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
        yoi_result->err->gc_refcount = 1;
        yoi_result->err->type_id = 0;
        yoi_result->err->value = result;
    } else {
        yoi_result->ok = (YoiUnsignedObject *)runtime_object_alloc(sizeof(YoiUnsignedObject));
        yoi_result->ok->gc_refcount = 1;
        yoi_result->ok->type_id = 0;
        yoi_result->ok->value = (unsigned long long)thread_id;
        yoi_result->err = nullptr;
    }

    // free the callable object
    if (--callable->gc_refcount == 0) {
        callable->gc_dec_func(callable->this_ptr);
        runtime_finalize_object((YoiObject *)callable);
    }
    return yoi_result;
}

YoiIntegerObject *runtime_thread_join(YoiUnsignedObject *thread_id) {
    auto pthread_id = (pthread_t)thread_id->value;
    auto result = pthread_join(pthread_id, nullptr);
    runtime_finalize_object((YoiObject *)thread_id);
    auto yoi_result = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 0;
    yoi_result->value = result;
    return yoi_result;
}
YoiUnsignedObject *runtime_get_thread_id() {
    pthread_t pthread_id = pthread_self();
    auto yoi_thread_id = (YoiUnsignedObject *)runtime_object_alloc(sizeof(YoiUnsignedObject));
    yoi_thread_id->gc_refcount = 1;
    yoi_thread_id->type_id = 0;
    yoi_thread_id->value = (unsigned long long)pthread_id;
    return yoi_thread_id;
}

YoiIntegerObject *runtime_ping_thread(YoiUnsignedObject *thread_id) {
    int result = pthread_kill((pthread_t)thread_id->value, 0);
    auto yoi_result = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 0;
    yoi_result->value = result;
    return yoi_result;
}
