#include <ctime>
#include <runtime/time/time.h>

#include <stdio.h>

#if defined (_WIN32)

long get_tm_gmtoff(struct tm *t) {
    struct tm temp = *t;

    time_t utc_timestamp = _mkgmtime(&temp);

    temp = *t;
    temp.tm_isdst = -1;

    time_t local_timestamp = mktime(&temp);

    return (long)(utc_timestamp - local_timestamp);
}

static struct tm *localtime_r(const time_t *timer, struct tm *buf) {
    if (timer == NULL || buf == NULL) {
        return NULL;
    }

    errno_t err = localtime_s(buf, timer);

    if (err != 0) {
        return NULL;
    }

    return buf;
}

static struct tm* gmtime_r(const time_t* timer, struct tm* buf) {
    if (timer == NULL || buf == NULL) {
        return NULL;
    }
    
    errno_t err = gmtime_s(buf, timer);
    
    if (err != 0) {
        return NULL;
    }
    
    return buf;
}

#if !defined (__CYGWIN__) && !defined (__MINGW32__)

#include <windows.h>

#define DELTA_EPOCH_IN_MICROSECS 116444736000000000LL

int clock_gettime(int clk_id, struct timespec *tp) {
    if (tp == NULL)
        return -1;

    if (clk_id == CLOCK_REALTIME) {
        FILETIME ft;
        GetSystemTimePreciseAsFileTime(&ft);

        uint64_t t = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;

        t -= DELTA_EPOCH_IN_MICROSECS;

        tp->tv_sec = (time_t)(t / 10000000);        // 100ns to seconds
        tp->tv_nsec = (long)((t % 10000000) * 100); // Remaining 100ns to ns

        return 0;
    } else if (clk_id == CLOCK_MONOTONIC) {
        static LARGE_INTEGER freq = {0};
        LARGE_INTEGER count;

        // Get frequency once (it doesn't change after boot)
        if (freq.QuadPart == 0) {
            if (!QueryPerformanceFrequency(&freq))
                return -1;
        }

        if (!QueryPerformanceCounter(&count))
            return -1;

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

    HANDLE timer = CreateWaitableTimerEx(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);

    if (!timer) {
        // Fallback for Win7/8 if Ex fails or flag unsupported
        timer = CreateWaitableTimer(NULL, TRUE, NULL);
        if (!timer)
            return -1;
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
#endif // !defined (__CYGWIN__) && !defined (__MINGW32__)

#else

long get_tm_gmtoff(struct tm *t) {
    return t->tm_gmtoff;
}

#endif // _WIN32

void YoiIntAndIntObject::gc_refcount_decrease(YoiIntAndIntObject *obj) {
    if (--obj->gc_refcount == 0) {
        runtime_finalize_object((YoiObject *)obj);
    }
}

YoiIntAndIntObject *YoiIntAndIntObject::create(int64_t seconds, int64_t nanoseconds) {
    auto *obj = (YoiIntAndIntObject *)runtime_object_alloc(sizeof(YoiIntAndIntObject));
    obj->gc_refcount = 1;
    obj->type_id = 0;
    obj->seconds = seconds;
    obj->nanoseconds = nanoseconds;
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
    struct tm tm{};
    gmtime_r(&t, &tm);
    char *result = (char *)malloc(MAX_TIME_STR_LEN);
    strftime(result, MAX_TIME_STR_LEN, format, &tm);
    return result;
}

int64_t runtime_time_localtimezone_offset() {
    time_t now = time(nullptr);
    tm local_time{};
    localtime_r(&now, &local_time);
    return get_tm_gmtoff(&local_time);
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
