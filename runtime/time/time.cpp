#include <runtime/time/time.h>

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>

#define DELTA_EPOCH_IN_MICROSECS  116444736000000000LL

int clock_gettime(int clk_id, struct timespec *tp) {
    if (tp == NULL) return -1;

    if (clk_id == CLOCK_REALTIME) {
        FILETIME ft;
        // Win8+ API for high precision. 
        // If you need Win7 support, use GetSystemTimeAsFileTime (less precise)
        GetSystemTimePreciseAsFileTime(&ft);

        // Merge DWORDs into a single 64-bit integer (100-nanosecond intervals)
        uint64_t t = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;

        // Subtract the 1601-1970 offset
        t -= DELTA_EPOCH_IN_MICROSECS;

        // Convert to seconds and nanoseconds
        tp->tv_sec = (time_t)(t / 10000000);            // 100ns to seconds
        tp->tv_nsec = (long)((t % 10000000) * 100);     // Remaining 100ns to ns

        return 0;
    } 
    else if (clk_id == CLOCK_MONOTONIC) {
        static LARGE_INTEGER freq = {0};
        LARGE_INTEGER count;

        // Get frequency once (it doesn't change after boot)
        if (freq.QuadPart == 0) {
            if (!QueryPerformanceFrequency(&freq)) return -1;
        }

        if (!QueryPerformanceCounter(&count)) return -1;

        // Convert QPC units to seconds and nanoseconds
        tp->tv_sec = (time_t)(count.QuadPart / freq.QuadPart);
        
        // Calculate remainder carefully to avoid overflow before division
        // logic: (count % freq) * 1e9 / freq
        tp->tv_nsec = (long)(((count.QuadPart % freq.QuadPart) * 1000000000) / freq.QuadPart);

        return 0;
    }

    return -1; // Unknown clock ID
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (req == NULL || req->tv_sec < 0 || req->tv_nsec < 0 || req->tv_nsec >= 1000000000) {
        return -1;
    }

    // Windows WaitableTimers use 100-nanosecond intervals.
    // Negative value indicates relative time.
    LARGE_INTEGER li;
    
    // Convert seconds + nanoseconds to 100ns ticks
    int64_t interval = req->tv_sec * 10000000LL + req->tv_nsec / 100;
    li.QuadPart = -interval; 

    // CREATE_WAITABLE_TIMER_HIGH_RESOLUTION requires Win 10 build 1803 or later.
    // If running on older Windows, fallback to 0 (standard resolution).
    #ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
    #define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
    #endif

    HANDLE timer = CreateWaitableTimerEx(NULL, NULL, 
        CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
    
    if (!timer) {
        // Fallback for Win7/8 if Ex fails or flag unsupported
        timer = CreateWaitableTimer(NULL, TRUE, NULL);
        if (!timer) return -1;
    }

    if (!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)) {
        CloseHandle(timer);
        return -1;
    }

    // Wait for the timer
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);

    // Note: Windows handles interruptions (APCs) differently than POSIX signals.
    // In a strict port, you would check if WaitForSingleObject returns WAIT_IO_COMPLETION
    // and calculate 'rem'. For most uses, this is unnecessary complexity.
    if (rem) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }

    return 0;
}
#endif

void YoiIntAndIntObject::gc_refcount_decrease(YoiIntAndIntObject *obj) {
    if (--obj->gc_refcount == 0) {
        if (--obj->seconds->gc_refcount == 0) {
            runtime_finalize_object((YoiObject *)obj->seconds);
        }
        if (--obj->nanoseconds->gc_refcount == 0) {
            runtime_finalize_object((YoiObject *)obj->nanoseconds);
        }
        runtime_finalize_object((YoiObject *)obj);
    }
}

YoiIntAndIntObject *YoiIntAndIntObject::create(int64_t seconds, int64_t nanoseconds) {
    YoiIntegerObject *seconds_obj = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    seconds_obj->gc_refcount = 1;
    seconds_obj->type_id = 0;
    seconds_obj->value = seconds;
    YoiIntegerObject *nanoseconds_obj = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    nanoseconds_obj->gc_refcount = 1;
    nanoseconds_obj->type_id = 0;
    nanoseconds_obj->value = nanoseconds;
    YoiIntAndIntObject *obj = (YoiIntAndIntObject *)runtime_object_alloc(sizeof(YoiIntAndIntObject));
    obj->gc_refcount = 1;
    obj->type_id = 0;
    obj->seconds = seconds_obj;
    obj->nanoseconds = nanoseconds_obj;
    return obj;
}

YoiIntAndIntObject *runtime_time_now() {
    struct timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    int64_t seconds = ts.tv_sec;
    int64_t nanoseconds = ts.tv_nsec;
    return YoiIntAndIntObject::create(seconds, nanoseconds);
}

void runtime_time_sleep(int64_t seconds, int64_t nanoseconds) {
    struct timespec ts{};
    ts.tv_sec = seconds;
    ts.tv_nsec = nanoseconds;
    nanosleep(&ts, nullptr);
}

char *runtime_time_strftime(const char *format, int64_t timestamp) {
    time_t t = timestamp;
    struct tm tm = *gmtime(&t); // UTC time
    char *result = (char *)malloc(MAX_TIME_STR_LEN);
    strftime(result, MAX_TIME_STR_LEN, format, &tm);
    return result;
}

int64_t runtime_time_localtimezone_offset() {
    time_t now = time(nullptr);
    tm local_time{};
    localtime_r(&now, &local_time);
    return local_time.tm_gmtoff;
}

YoiIntAndIntObject *runtime_time_monotonic_now() {
    struct timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    int64_t seconds = ts.tv_sec;
    int64_t nanoseconds = ts.tv_nsec;
    return YoiIntAndIntObject::create(seconds, nanoseconds);
}

void runtime_time_finalize(void *memory) {
    free(memory);
}
