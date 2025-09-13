#include "runtime/threading/threading.h"
#include "runtime/memory/memory.h"
#include <errno.h>

struct ThreadStarterArgs {
    YoiVoidCallableInterface *callable;
};

#ifdef _WIN32 // Windows Implementation

unsigned __stdcall thread_starter_wrapper(void* args) {
    auto* starter_args = (ThreadStarterArgs*)args;
    YoiVoidCallableInterface* callable = starter_args->callable;
    
    callable->callable(callable->this_ptr);

    // Now that the thread's work is done, we can release the callable.
    if (--callable->gc_refcount == 0) {
        if (callable->gc_dec_func) {
            callable->gc_dec_func(callable->this_ptr);
        }
        runtime_finalize_object((YoiObject *)callable);
    }

    // Free the wrapper args struct
    free(starter_args);

    return 0;
}

YoiResultUnsignedAndIntObject *runtime_start_thread(YoiVoidCallableInterface *callable) {
    auto* starter_args = (ThreadStarterArgs*) malloc(sizeof(ThreadStarterArgs));
    starter_args->callable = callable;

    uintptr_t thread_handle = _beginthreadex(nullptr, 0, &thread_starter_wrapper, starter_args, 0, nullptr);

    auto yoi_result = (YoiResultUnsignedAndIntObject *)runtime_object_alloc(sizeof(YoiResultUnsignedAndIntObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 16;

    if (thread_handle == 0) {
        yoi_result->ok = nullptr;
        yoi_result->err = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
        yoi_result->err->gc_refcount = 1;
        yoi_result->err->type_id = 0;
        yoi_result->err->value = errno;
        // We must clean up the args if thread creation failed
        free(starter_args);
    } else {
        yoi_result->err = nullptr;
        yoi_result->ok = (YoiUnsignedObject *)runtime_object_alloc(sizeof(YoiUnsignedObject));
        yoi_result->ok->gc_refcount = 1;
        yoi_result->ok->type_id = 0;
        yoi_result->ok->value = (unsigned long long)thread_handle;
    }

    // IMPORTANT: the refcount for 'callable' is NOT decremented here.
    // its ownership is passed to the new thread via the wrapper.
    return yoi_result;
}

YoiIntegerObject *runtime_thread_join(YoiUnsignedObject *thread_handle_obj) {
    auto win_handle = (YoiThreadHandle)thread_handle_obj->value;
    
    DWORD result = WaitForSingleObject(win_handle, INFINITE);
    
    CloseHandle(win_handle);

    auto yoi_result = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 0;
    
    if (result == WAIT_OBJECT_0) {
        yoi_result->value = 0;
    } else {
        yoi_result->value = GetLastError();
    }
    
    if (--thread_handle_obj->gc_refcount == 0)
        runtime_finalize_object((YoiObject *)thread_handle_obj);
    return yoi_result;
}

YoiUnsignedObject *runtime_get_thread_id() {
    YoiThreadId thread_id = GetCurrentThreadId();
    auto yoi_thread_id = (YoiUnsignedObject *)runtime_object_alloc(sizeof(YoiUnsignedObject));
    yoi_thread_id->gc_refcount = 1;
    yoi_thread_id->type_id = 0;
    yoi_thread_id->value = (unsigned long long)thread_id;
    return yoi_thread_id;
}

YoiIntegerObject *runtime_ping_thread(YoiUnsignedObject *thread_handle_obj) {
    auto win_handle = (YoiThreadHandle)thread_handle_obj->value;
    DWORD exit_code;

    auto yoi_result = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 0;

    if (GetExitCodeThread(win_handle, &exit_code)) {
        if (exit_code == STILL_ACTIVE) {
            yoi_result->value = 0;
        } else {
            yoi_result->value = ESRCH;
        }
    } else {
        yoi_result->value = ESRCH;
    }
    
    if (--thread_handle_obj->gc_refcount == 0)
        runtime_finalize_object((YoiObject *)thread_handle_obj);
    return yoi_result;
}

#else // POSIX (-nix) Implementation

void* thread_starter_wrapper(void* args) {
    auto* starter_args = (ThreadStarterArgs*)args;
    YoiVoidCallableInterface* callable = starter_args->callable;

    callable->callable(callable->this_ptr);

    // now that the thread's work is done, we can release the callable.
    if (--callable->gc_refcount == 0) {
        if (callable->gc_dec_func) {
            callable->gc_dec_func(callable->this_ptr);
        }
        runtime_finalize_object((YoiObject *)callable);
    }
    
    free(starter_args);
    
    return nullptr;
}

YoiResultUnsignedAndIntObject *runtime_start_thread(YoiVoidCallableInterface *callable) {
    auto* starter_args = (ThreadStarterArgs*) malloc(sizeof(ThreadStarterArgs));
    starter_args->callable = callable;

    YoiThreadHandle thread_id;
    int result = pthread_create(&thread_id, nullptr, &thread_starter_wrapper, starter_args);

    auto yoi_result = (YoiResultUnsignedAndIntObject *)runtime_object_alloc(sizeof(YoiResultUnsignedAndIntObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 16;

    if (result != 0) {
        yoi_result->ok = nullptr;
        yoi_result->err = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
        yoi_result->err->gc_refcount = 1;
        yoi_result->err->type_id = 0;
        yoi_result->err->value = result;
        free(starter_args);
    } else {
        yoi_result->err = nullptr;
        yoi_result->ok = (YoiUnsignedObject *)runtime_object_alloc(sizeof(YoiUnsignedObject));
        yoi_result->ok->gc_refcount = 1;
        yoi_result->ok->type_id = 0;
        yoi_result->ok->value = (unsigned long long)thread_id;
    }
    
    // IMPORTANT: the refcount for 'callable' is NOT decremented here.
    // its ownership is passed to the new thread via the wrapper.
    return yoi_result;
}

YoiIntegerObject *runtime_thread_join(YoiUnsignedObject *thread_id_obj) {
    auto pthread_id = (YoiThreadHandle)thread_id_obj->value;
    int result = pthread_join(pthread_id, nullptr);
    
    auto yoi_result = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 0;
    yoi_result->value = result;

    if (--thread_id_obj->gc_refcount == 0)
        runtime_finalize_object((YoiObject *)thread_id_obj);
    return yoi_result;
}

YoiUnsignedObject *runtime_get_thread_id() {
    YoiThreadId pthread_id = pthread_self();
    auto yoi_thread_id = (YoiUnsignedObject *)runtime_object_alloc(sizeof(YoiUnsignedObject));
    yoi_thread_id->gc_refcount = 1;
    yoi_thread_id->type_id = 0;
    yoi_thread_id->value = (unsigned long long)pthread_id;
    return yoi_thread_id;
}

YoiIntegerObject *runtime_ping_thread(YoiUnsignedObject *thread_id_obj) {
    int result = pthread_kill((YoiThreadHandle)thread_id_obj->value, 0);
    auto yoi_result = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 0;
    yoi_result->value = result;
    
    if (--thread_id_obj->gc_refcount == 0)
        runtime_finalize_object((YoiObject *) thread_id_obj);
    return yoi_result;
}

#endif // _WIN32