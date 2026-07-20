//
// Created by XIaokang00010 on 2026/7/20.
//

#ifndef HOSHI_BACON_HPP
#define HOSHI_BACON_HPP

#include <runtime/build_config.h>
#include <runtime/memory/memory.h>
#include <mimalloc/include/mimalloc.h>
#include <runtime/bacon/wait.h>
#include <runtime/bacon/small_vector.h>

#if defined(__x86_64__) || defined(_M_X64)
  #include <immintrin.h>
  #define CPU_RELAX() _mm_pause()
#elif defined(__aarch64__) || defined(__arm__)
  #define CPU_RELAX() asm volatile("yield" ::: "memory")
#else
  #define CPU_RELAX()
#endif
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

/**
 * @brief Local buffer node for the local buffer. Each node contains a fixed 
 *        number of entries (ELYSIA_BACON_LOCAL_BUFFER_SIZE) and a reference 
 *        count. The local buffer is used to store objects that are not yet 
 *        finalized and are not yet part of the global buffer. 
 *        The local buffer is thread-local, meaning that each thread has its 
 *        own local buffer.
 *        The local buffer is used to reduce the overhead of allocating and 
 *        deallocating objects in the global buffer. 
 * 
 */
struct LocalBufferNode {
    unsigned long long count;
    YoiObject *entries[ELYSIA_BACON_LOCAL_BUFFER_SIZE];
};

extern thread_local LocalBufferNode local_buffer;

struct BufferNode {
    YoiObject *entries[ELYSIA_BACON_LOCAL_BUFFER_SIZE];

    BufferNode *next;
};

extern BufferNode* global_buffer_head;
extern unsigned int current_global_buffer_page_count;
extern unsigned int local_thread_should_sleep;
extern int bacon_current_active_threads;
extern pthread_t bacon_thread_handle;

extern "C" void bacon_flush_local_buffer();

extern "C" void bacon_flush_global_buffer();

extern "C" void bacon_push_to_local_buffer(YoiObject *object);

extern "C" void bacon_poll();

void bacon_mark_grey(YoiObject *object);

void bacon_scan_black(YoiObject *obj);

void bacon_scan(YoiObject *obj);

void bacon_collect_white(SmallVector &result, YoiObject *obj);

extern "C" void bacon_stw();

void *bacon_recycler_thread(void *args);

extern "C" void bacon_init();

extern "C" void bacon_enter_ffi();
extern "C" void bacon_leave_ffi();

#endif // HOSHI_BACON_HPP