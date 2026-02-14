//
// Created by XIaokang00010 on 2025/11/23.
//

#ifndef HOSHI_LANG_TIME_H
#define HOSHI_LANG_TIME_H

#include "runtime/memory/memory.h"
#include <cstdint>
#include <ctime>

#ifdef _WIN32
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1

int clock_gettime(int clk_id, struct timespec *tp);
int nanosleep(const struct timespec *req, struct timespec *rem);
#endif

#define MAX_TIME_STR_LEN 128


struct YoiIntAndIntObject {
    unsigned long long gc_refcount;
    unsigned long long type_id;
    unsigned long long seconds;
    unsigned long long nanoseconds;

    static void gc_refcount_decrease(YoiIntAndIntObject *obj);

    static YoiIntAndIntObject *create(int64_t seconds, int64_t nanoseconds);
};

#define LIBTIME_EXPORT extern "C"
#define LIBTIME_NOFFI
#define LIBTIME_FFI 

/**
 * @brief Return the current time as a YoiIntAndIntObject.
 * 
 * @return YoiIntAndIntObject* The current time as a YoiIntAndIntObject.
 * @note The type-id of the returned object will be audited by the runtime.
 */
LIBTIME_EXPORT LIBTIME_NOFFI YoiIntAndIntObject *runtime_time_now();

/**
 * @brief Sleep for a given number of seconds.
 * 
 * @param seconds The number of seconds to sleep.
 */
LIBTIME_EXPORT LIBTIME_FFI void runtime_time_sleep(int64_t seconds, int64_t nanoseconds);

/**
 * @brief Convert a timestamp to a string according to a given format.
 * 
 * @param format The format string.
 * @param timestamp The timestamp to convert.
 * @return char* The resulting string.
 */
LIBTIME_EXPORT LIBTIME_FFI char *runtime_time_strftime(const char *format, int64_t timestamp);

/**
 * @brief Get the local timezone offset in seconds.
 * 
 * @return int64_t The local timezone offset in seconds.
 */
LIBTIME_EXPORT LIBTIME_FFI int64_t runtime_time_localtimezone_offset();

/**
 * @brief Get the current monotonic time as a YoiIntAndIntObject.
 * 
 * @return YoiIntAndIntObject* The current monotonic time as a YoiIntAndIntObject.
 * @note The type-id of the returned object will be audited by the runtime.
 */
LIBTIME_EXPORT LIBTIME_NOFFI YoiIntAndIntObject *runtime_time_monotonic_now();

/**
 * @brief Finalize the memory created by the time module.
 * 
 * @param memory The memory to finalize.
 */
LIBTIME_EXPORT LIBTIME_FFI void runtime_time_finalize(void *memory);

#endif //HOSHI_LANG_TIME_H