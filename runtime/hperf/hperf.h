//
// Created by XIaokang00010 on 2026/1/1.
//

#ifndef HOSHI_LANG_HPERF_H
#define HOSHI_LANG_HPERF_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <runtime/fs/fs.h>
#include <runtime/time/time.h>
#include <runtime/threading/threading.h>

enum class HPerfEventType : uint8_t {
    FUNC_ENTER = 0,
    FUNC_LEAVE,
    MEM_ALLOC,
    MEM_FREE,
};

struct HPerfTimestamp {
    uint64_t sec;
    uint64_t nsec;
};

struct HPerfEvent {
    HPerfEventType type;
    HPerfTimestamp timestamp;
    void *data[2]; // whatever it is for the event type, just a symbol for the rest of the data
};

struct HPerfEventFuncEnter {
    const char *func_name;
};

struct HPerfEventFuncLeave {
    const char *func_name;
};

struct HPerfEventMemAlloc {
    void *ptr;
    size_t size;
};

struct HPerfEventMemFree {
    void *ptr;
};

struct HPerfPage {
    HPerfEvent data[1024];
    size_t num_events;
    HPerfPage *next;
};

struct HPerfContext {
    HPerfContext *prev;
    HPerfContext *next;
    size_t unique_id; // identify each thread, in case multiple threads are running
    HPerfPage *pages;
    HPerfPage *current_page;
    size_t num_events;
};

extern HPerfContext *hperf_contexts;

extern bool hperf_enabled;

extern char hperf_report_filename[2048];

HPerfContext *hperf_context_create(size_t unique_id);

void hperf_context_finalize(HPerfContext *ctx);

HPerfContext *hperf_context_get(size_t unique_id);

HPerfEvent *hperf_context_add_event(HPerfContext *ctx);

void hperf_report_func_enter(const char *func_name);

void hperf_report_func_leave(const char *func_name);

void hperf_report_mem_alloc(void *ptr, size_t size);

void hperf_report_mem_free(void *ptr);

void hperf_write_report(const char *filename);

void hperf_init(const char *filename);

#endif // HOSHI_LANG_HPERF_H