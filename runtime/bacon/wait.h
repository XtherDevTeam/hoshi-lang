//
// Created by XIaokang00010 on 2026/7/20.
//

#include <cstdint>

#if defined(__linux__)
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>

inline void wait_on_address(unsigned int* addr, unsigned int expected) {
    syscall(SYS_futex, addr, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, expected, NULL, NULL, 0);
}
inline void wake_all_on_address(unsigned int* addr) {
    syscall(SYS_futex, addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT32_MAX, NULL, NULL, 0);
}


#elif defined(_WIN32)
// for Windows, use Windows API
#include <windows.h>

inline void wait_on_address(unsigned int* addr, unsigned int expected) {
    WaitOnAddress(addr, &expected, sizeof(unsigned int), INFINITE);
}
inline void wake_all_on_address(unsigned int* addr) {
    WakeByAddressAll(addr);
}

#else
// for other platforms, use pthreads
#include <pthread.h>

extern pthread_mutex_t g_safepoint_mutex;
extern pthread_cond_t g_safepoint_cond;

inline void wait_on_address(unsigned int* addr, unsigned int expected) {
    pthread_mutex_lock(&g_safepoint_mutex);
    while (__atomic_load_n(addr, __ATOMIC_RELAXED) == expected) {
        pthread_cond_wait(&g_safepoint_cond, &g_safepoint_mutex);
    }
    pthread_mutex_unlock(&g_safepoint_mutex);
}

inline void wake_all_on_address(unsigned int* addr) {
    pthread_mutex_lock(&g_safepoint_mutex);
    pthread_cond_broadcast(&g_safepoint_cond);
    pthread_mutex_unlock(&g_safepoint_mutex);
}

#endif