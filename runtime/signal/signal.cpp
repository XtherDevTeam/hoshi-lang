#include "runtime/memory/memory.h"
#include <csignal>
#include <runtime/signal/signal.h>

runtime_signal_handler_info *runtime_signal_handlers[3];

// ---------------------------------------------------------
// Windows Helper for SIGALRM Emulation
// ---------------------------------------------------------
#ifdef _WIN32
// Global state for the simulated alarm timer
static HANDLE hTimerQueue = NULL;
static HANDLE hTimer = NULL;
static runtime_signal_handler_t g_alarm_handler = NULL;

VOID CALLBACK WindowsAlarmCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
    if (g_alarm_handler) {
        g_alarm_handler(HS_SIGALRM);
    }
}

void cleanup_win_timer() {
    if (hTimer) {
        DeleteTimerQueueTimer(hTimerQueue, hTimer, NULL);
        hTimer = NULL;
    }
    if (hTimerQueue) {
        DeleteTimerQueue(hTimerQueue);
        hTimerQueue = NULL;
    }
}
#endif

LIBSIG_EXPORT YoiIntegerObject *runtime_signal_register(YoiIntegerObject *signum, YoiVoidIntCallableInterface *handler) {
    if (signum->value == HS_SIGKILL) {
        // SIGKILL cannot be caught, blocked, or ignored on any platform.
        YoiIntegerObject *err = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
        err->type_id = 0;
        err->gc_refcount = 1;
        err->value = LIBSIG_ERR_CANT_CATCH_KILL;
        return err;
    }

    int type = -1;
    switch (signum->value) {
        case HS_SIGINT:
            type = HANDLER_TYPE_SIGINT;
            break;
        case HS_SIGTERM:
            type = HANDLER_TYPE_SIGTERM;
            break;
        case HS_SIGALRM:
            type = HANDLER_TYPE_SIGALRM;
            break;
        default:
            break;
    }

    if (type == -1) {
        YoiIntegerObject *err = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
        err->type_id = 0;
        err->gc_refcount = 1;
        err->value = LIBSIG_ERR_UNSUPPORTED;
        return err;
    }

    runtime_signal_handler_info *info = (runtime_signal_handler_info *)malloc(sizeof(runtime_signal_handler_info)); // not YoiObject, use malloc
    info->handler = handler->handler;
    info->next = runtime_signal_handlers[type];
    runtime_signal_handlers[type] = info;

    if (--handler->gc_refcount == 0) {
        if (handler->gc_dec_func) {
            handler->gc_dec_func(handler->this_ptr);
        }
        runtime_finalize_object((YoiObject *)handler);
    }

    YoiIntegerObject *success = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    success->type_id = 0;
    success->gc_refcount = 1;
    success->value = LIBSIG_SUCCESS;
    return success;
}

LIBSIG_EXPORT int runtime_signal_alarm(unsigned int seconds) {
#ifdef _WIN32
    // Initialize Queue if needed
    if (!hTimerQueue) {
        hTimerQueue = CreateTimerQueue();
        if (!hTimerQueue) return LIBSIG_ERR_UNSUPPORTED;
    }

    // Clear existing timer if exists
    if (hTimer) {
        DeleteTimerQueueTimer(hTimerQueue, hTimer, NULL);
        hTimer = NULL;
    }

    // If seconds is 0, we just wanted to cancel (done above)
    if (seconds == 0) {
        return 0; // equivalent to returning remaining seconds, simplified here to 0
    }

    // Create new timer
    // seconds * 1000 for milliseconds
    if (!CreateTimerQueueTimer(&hTimer, hTimerQueue, 
            (WAITORTIMERCALLBACK)WindowsAlarmCallback, 
            NULL, seconds * 1000, 0, WT_EXECUTEONLYONCE)) {
        return LIBSIG_ERR_UNSUPPORTED;
    }

    return 0; 
#else
    // POSIX Implementation
    return alarm(seconds);
#endif
}

LIBSIG_EXPORT int runtime_signal_raise(int signum) {
    // Special handling for SIGKILL
    if (signum == HS_SIGKILL) {
#ifdef _WIN32
        // Windows doesn't strictly support raising SIGKILL via raise(), 
        // so we force termination.
        TerminateProcess(GetCurrentProcess(), 1);
        return LIBSIG_SUCCESS; // Unreachable, strictly speaking
#else
        kill(getpid(), SIGKILL);
        return LIBSIG_SUCCESS;
#endif
    }

#ifdef _WIN32
    // Windows raise() doesn't support custom signals like our emulated ALRM
    if (signum == HS_SIGALRM) {
        if (g_alarm_handler) {
            g_alarm_handler(HS_SIGALRM);
            return LIBSIG_SUCCESS;
        }
        return LIBSIG_ERR_RAISE_FAIL;
    }
#endif

    if (raise(signum) != 0) {
        return LIBSIG_ERR_RAISE_FAIL;
    }
    return LIBSIG_SUCCESS;
}

LIBSIG_EXPORT int runtime_signal_default(int signum) {
    if (signum == HS_SIGKILL) return LIBSIG_ERR_CANT_CATCH_KILL;

#ifdef _WIN32
    if (signum == HS_SIGALRM) {
        while (runtime_signal_handlers[HANDLER_TYPE_SIGALRM]->next) {
            auto next = runtime_signal_handlers[HANDLER_TYPE_SIGALRM]->next;
            free(runtime_signal_handlers[HANDLER_TYPE_SIGALRM]);
            runtime_signal_handlers[HANDLER_TYPE_SIGALRM] = next;
        }
        return LIBSIG_SUCCESS;
    }
#endif

    while (runtime_signal_handlers[signum]->next) {
        auto next = runtime_signal_handlers[signum]->next;
        free(runtime_signal_handlers[signum]);
        runtime_signal_handlers[signum] = next;
    }
    
    return LIBSIG_SUCCESS;
}

LIBSIG_EXPORT int runtime_signal_ignore(int signum) {
    if (signum == HS_SIGKILL) return LIBSIG_ERR_CANT_CATCH_KILL;

#ifdef _WIN32
    if (signum == HS_SIGALRM) {
        g_alarm_handler = NULL; // If we clear the handler, the timer fires but does nothing
        return LIBSIG_SUCCESS;
    }
#endif

    if (signal(signum, SIG_IGN) == SIG_ERR) {
        return LIBSIG_ERR_REGISTER_FAIL;
    }
    return LIBSIG_SUCCESS;
}

void runtime_signal_handler(int signum) {
    YoiIntegerObject *signum_obj = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    signum_obj->type_id = 0;
    signum_obj->gc_refcount = 1;
    signum_obj->value = signum;

    switch (signum) {
        case HS_SIGINT:
            for (runtime_signal_handler_info *info = runtime_signal_handlers[HANDLER_TYPE_SIGINT]; info; info = info->next) {
                info->handler(signum_obj);
            }
            break;
        case HS_SIGTERM:
            for (runtime_signal_handler_info *info = runtime_signal_handlers[HANDLER_TYPE_SIGTERM]; info; info = info->next) {
                info->handler(signum_obj);
            }
            break;
        case HS_SIGALRM:
            for (runtime_signal_handler_info *info = runtime_signal_handlers[HANDLER_TYPE_SIGALRM]; info; info = info->next) {
                info->handler(signum_obj);
            }
            break;
        default:
            break;
    }
}

LIBSIG_EXPORT void runtime_signal_init() {
    // Install our signal handler
    #ifdef _WIN32
    g_alarm_handler = runtime_signal_handler;
    #else
    signal(LIBSIG_INTERNAL_ALRM, runtime_signal_handler);
    signal(LIBSIG_INTERNAL_KILL, runtime_signal_handler);
    signal(SIGPIPE, SIG_IGN);
    #endif
    signal(SIGINT, runtime_signal_handler);
    signal(SIGTERM, runtime_signal_handler);
}
