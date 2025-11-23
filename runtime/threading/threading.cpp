#include "runtime/threading/threading.h"
#include "runtime/memory/memory.h"
#include <errno.h>

struct ThreadStarterArgs {
    YoiVoidCallableInterface *callable;
};

#ifdef _WIN32 // Windows Implementation

#include <windows.h>

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

YoiResultUnsignedAndIntObject *runtime_thread_new_mutex_lock() {
    auto* cs = (CRITICAL_SECTION*) malloc(sizeof(CRITICAL_SECTION));
    if (!cs) {
        auto yoi_result_obj = (YoiResultUnsignedAndIntObject *)runtime_object_alloc(sizeof(YoiResultUnsignedAndIntObject));
        yoi_result_obj->gc_refcount = 1;
        yoi_result_obj->type_id = 16;
        yoi_result_obj->ok = nullptr;
        
        yoi_result_obj->err = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
        yoi_result_obj->err->gc_refcount = 1;
        yoi_result_obj->err->type_id = 0;
        yoi_result_obj->err->value = ENOMEM;
        return yoi_result_obj;
    }

    InitializeCriticalSection(cs);

    auto yoi_result = (YoiUnsignedObject *)runtime_object_alloc(sizeof(YoiUnsignedObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 0;
    yoi_result->value = (unsigned long long)cs;

    auto yoi_result_obj = (YoiResultUnsignedAndIntObject *)runtime_object_alloc(sizeof(YoiResultUnsignedAndIntObject));
    yoi_result_obj->gc_refcount = 1;
    yoi_result_obj->type_id = 16;
    yoi_result_obj->err = nullptr;
    yoi_result_obj->ok = yoi_result;

    return yoi_result_obj;
}

void runtime_thread_finalize_mutex_lock(YoiUnsignedObject *handle) {
    auto *cs = (CRITICAL_SECTION *)handle->value;
    DeleteCriticalSection(cs);
    free(cs);

    if (--handle->gc_refcount == 0)
        runtime_finalize_object((YoiObject *)handle);
}

void runtime_thread_mutex_lock(YoiUnsignedObject *mutex_handle) {
    EnterCriticalSection((CRITICAL_SECTION *)mutex_handle->value);
    
    if (--mutex_handle->gc_refcount == 0)
        runtime_finalize_object((YoiObject *)mutex_handle);
}

void runtime_thread_mutex_unlock(YoiUnsignedObject *mutex_handle) {
    LeaveCriticalSection((CRITICAL_SECTION *)mutex_handle->value);

    if (--mutex_handle->gc_refcount == 0)
        runtime_finalize_object((YoiObject *)mutex_handle);
}

YoiIntegerObject *runtime_thread_mutex_try_lock(YoiUnsignedObject *mutex_handle) {
    BOOL res = TryEnterCriticalSection((CRITICAL_SECTION *)mutex_handle->value);
    
    auto yoi_result = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 0;
    yoi_result->value = res ? 0 : EBUSY;

    if (--mutex_handle->gc_refcount == 0)
        runtime_finalize_object((YoiObject *)mutex_handle);
    
    return yoi_result;
}

#else // POSIX (-nix) Implementation

#include <pthread.h>

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

YoiResultUnsignedAndIntObject *runtime_thread_new_mutex_lock() {
    auto mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    auto result = pthread_mutex_init(mutex, nullptr);

    if (result == 0) {
        auto yoi_result = (YoiUnsignedObject *)runtime_object_alloc(sizeof(YoiUnsignedObject));
        yoi_result->gc_refcount = 1;
        yoi_result->type_id = 0;
        yoi_result->value = (unsigned long long)mutex;

        auto yoi_result_obj = (YoiResultUnsignedAndIntObject *)runtime_object_alloc(sizeof(YoiResultUnsignedAndIntObject));
        yoi_result_obj->gc_refcount = 1;
        yoi_result_obj->err = nullptr;
        yoi_result_obj->ok = yoi_result;
        return yoi_result_obj;
    } else {
        auto yoi_result = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
        yoi_result->gc_refcount = 1;
        yoi_result->type_id = 0;
        yoi_result->value = 0;

        auto yoi_result_obj = (YoiResultUnsignedAndIntObject *)runtime_object_alloc(sizeof(YoiResultUnsignedAndIntObject));
        yoi_result_obj->gc_refcount = 1;
        yoi_result_obj->err = yoi_result;
        yoi_result_obj->ok = nullptr;

        return yoi_result_obj;
    }
    
}

void runtime_thread_finalize_mutex_lock(YoiUnsignedObject *handle) {
    auto *mutex = (pthread_mutex_t *)handle->value;
    pthread_mutex_destroy(mutex);

    if (--handle->gc_refcount == 0)
        runtime_finalize_object((YoiObject *)handle);
}

void runtime_thread_mutex_lock(YoiUnsignedObject *mutex_handle) {
    pthread_mutex_lock((pthread_mutex_t *)mutex_handle->value);
    
    if (--mutex_handle->gc_refcount == 0)
        runtime_finalize_object((YoiObject *)mutex_handle);
}

void runtime_thread_mutex_unlock(YoiUnsignedObject *mutex_handle) {
    pthread_mutex_unlock((pthread_mutex_t *)mutex_handle->value);

    if (--mutex_handle->gc_refcount == 0)
        runtime_finalize_object((YoiObject *)mutex_handle);
}

YoiIntegerObject *runtime_thread_mutex_try_lock(YoiUnsignedObject *mutex_handle) {
    auto res = pthread_mutex_trylock((pthread_mutex_t *)mutex_handle->value);
    auto yoi_result = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    yoi_result->gc_refcount = 1;
    yoi_result->type_id = 0;
    yoi_result->value = res;

    if (--mutex_handle->gc_refcount == 0)
        runtime_finalize_object((YoiObject *)mutex_handle);
    
    return yoi_result;
}
#endif // _WIN32