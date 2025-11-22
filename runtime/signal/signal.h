//
// Created by XIaokang00010 on 2025/11/02.
//

#ifndef HOSHILANGSTD_LIBSIG
#define HOSHILANGSTD_LIBSIG

#include <csignal>
#include <runtime/memory/memory.h>

#ifdef _WIN32
#include <windows.h>
// Windows doesn't define SIGALRM, we define a custom ID for our emulation
#define LIBSIG_INTERNAL_ALRM 14
#define LIBSIG_INTERNAL_KILL 9
#else
#include <unistd.h>
#include <sys/types.h>
#define LIBSIG_INTERNAL_ALRM SIGALRM
#define LIBSIG_INTERNAL_KILL SIGKILL
#endif

#define LIBSIG_EXPORT extern "C"

// Error Codes
#define LIBSIG_SUCCESS 0
#define LIBSIG_ERR_REGISTER_FAIL -1
#define LIBSIG_ERR_UNSUPPORTED -2
#define LIBSIG_ERR_CANT_CATCH_KILL -3
#define LIBSIG_ERR_RAISE_FAIL -4

#define HS_SIGINT  SIGINT
#define HS_SIGTERM SIGTERM
#define HS_SIGALRM LIBSIG_INTERNAL_ALRM
#define HS_SIGKILL LIBSIG_INTERNAL_KILL

#define HANDLER_TYPE_SIGINT 0
#define HANDLER_TYPE_SIGTERM 1
#define HANDLER_TYPE_SIGALRM 2

typedef void (*runtime_signal_handler_bare_t)(int signum);
typedef void (*runtime_signal_handler_t)(YoiObject *self, YoiIntegerObject *signum);

struct YoiVoidIntCallableInterface {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    void *this_ptr;
    void *(*gc_inc_func)(void *this_ptr);
    void *(*gc_dec_func)(void *this_ptr);
    runtime_signal_handler_t handler;
};

struct runtime_signal_handler_info {
    YoiVoidIntCallableInterface *self;
    runtime_signal_handler_info *next;
};

extern runtime_signal_handler_info *runtime_signal_handlers[3];

void runtime_signal_handler(int signum);

LIBSIG_EXPORT void runtime_signal_init();

// Register a handler for a specific signal
// Returns 0 on success, or negative error code
LIBSIG_EXPORT YoiIntegerObject *runtime_signal_register(YoiIntegerObject *signum, YoiVoidIntCallableInterface *handler);

// Schedule an alarm signal to be delivered after seconds
// Passing 0 cancels any pending alarm.
LIBSIG_EXPORT int runtime_signal_alarm(unsigned int seconds);

// Raise a signal to the current process
LIBSIG_EXPORT int runtime_signal_raise(int signum);

// Restore default handling for a signal
LIBSIG_EXPORT int runtime_signal_default(int signum);

// Ignore a signal
LIBSIG_EXPORT int runtime_signal_ignore(int signum);

#endif // HOSHILANGSTD_LIBSIG