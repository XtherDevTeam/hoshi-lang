//
// Created by XIaokang00010 on 2026/1/1.
//

#include "runtime/threading/threading.h"
#include <cstdio>
#include <cstring>
#include <runtime/hperf/hperf.h>

HPerfContext *hperf_contexts = nullptr;

bool hperf_enabled = false;

char hperf_report_filename[2048];

HPerfContext *hperf_context_create(size_t unique_id) {
    // use malloc to allocate the context, so that we don't need to worry about memory management
    HPerfContext *ctx = static_cast<HPerfContext *>(malloc(sizeof(HPerfContext)));
    ctx->unique_id = unique_id;
    ctx->pages = nullptr;
    ctx->current_page = nullptr;
    ctx->num_events = 0;
    ctx->prev = nullptr;
    ctx->next = hperf_contexts;
    if (hperf_contexts != nullptr) {
        hperf_contexts->prev = ctx;
    }
    hperf_contexts = ctx;
    return ctx;
}

void hperf_context_finalize(HPerfContext *ctx) {
    // free all pages in the context
    HPerfPage *page = ctx->pages;
    while (page != nullptr) {
        HPerfPage *next = page->next;
        free(page);
        page = next;
    }
    // remove the context from the list
    if (ctx->next != nullptr) {
        ctx->next->prev = ctx->prev;
    }
    if (ctx->prev != nullptr) {
        ctx->prev->next = ctx->next;
    } else {
        hperf_contexts = ctx->next;
    }
    free(ctx);
}

HPerfEvent *hperf_context_add_event(HPerfContext *ctx) {
    if (ctx->current_page == nullptr || ctx->current_page->num_events == 1024) {
        // allocate a new page if the current one is full
        auto page = static_cast<HPerfPage *>(calloc(sizeof(HPerfPage), 1));
        page->next = nullptr;
        if (ctx->current_page != nullptr) {
            ctx->current_page->next = page;
        } else {
            ctx->pages = page;
        }
        ctx->current_page = page;
    }
    HPerfEvent *event = &ctx->current_page->data[ctx->current_page->num_events++];
    ctx->num_events++;
    struct timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    event->timestamp.sec = ts.tv_sec;
    event->timestamp.nsec = ts.tv_nsec;
    return event;
}

void hperf_report_func_enter(const char *func_name) {
    auto ctx = hperf_context_get(runtime_get_thread_id());
    if (ctx == nullptr) {
        return;
    }
    auto event = hperf_context_add_event(ctx);
    event->type = HPerfEventType::HPERF_EVT_FUNC_ENTER;
    auto data = reinterpret_cast<HPerfEventFuncEnter *>(event->data);
    data->func_name = func_name;
}

void hperf_report_func_leave(const char *func_name) {
    auto ctx = hperf_context_get(runtime_get_thread_id());
    if (ctx == nullptr) {
        return;
    }
    auto event = hperf_context_add_event(ctx);
    event->type = HPerfEventType::HPERF_EVT_FUNC_LEAVE;
    auto data = reinterpret_cast<HPerfEventFuncLeave *>(event->data);
    data->func_name = func_name;
}

void hperf_report_mem_alloc(void *ptr, size_t size) {
    auto ctx = hperf_context_get(runtime_get_thread_id());
    if (ctx == nullptr) {
        return;
    }
    auto event = hperf_context_add_event(ctx);
    event->type = HPerfEventType::HPERF_EVT_MEM_ALLOC;
    auto data = reinterpret_cast<HPerfEventMemAlloc *>(event->data);
    data->ptr = ptr;
    data->size = size;
}

void hperf_report_mem_free(void *ptr) {
    auto ctx = hperf_context_get(runtime_get_thread_id());
    if (ctx == nullptr) {
        return;
    }
    auto event = hperf_context_add_event(ctx);
    event->type = HPerfEventType::HPERF_EVT_MEM_FREE;
    auto data = reinterpret_cast<HPerfEventMemFree *>(event->data);
    data->ptr = ptr;
}

void hperf_write_report(const char *filename) {
    for (HPerfContext *ctx = hperf_contexts; ctx != nullptr; ctx = ctx->next) {
        char real_filename[2048];
        snprintf(real_filename, 2048, "%s.%lu.hperf.json", filename, ctx->unique_id);
        FILE *f = fopen(real_filename, "w");
        if (f == nullptr) {
            return;
        }
        // we use json
        fprintf(f,
                R"({"filetype": "hperf-report", "version": 1, "unique-id": %lu, "num-events": %lu, "events": [)",
                ctx->unique_id,
                ctx->num_events);
        bool first = true;
        for (HPerfContext *c = hperf_contexts; c != nullptr; c = c->next) {
            for (HPerfPage *p = c->pages; p != nullptr; p = p->next) {
                for (size_t i = 0; i < p->num_events; i++) {
                    auto event = &p->data[i];
                    if (first) {
                        first = false;
                    } else {
                        fprintf(f, ",");
                    }
                    switch (event->type) {
                        case HPerfEventType::HPERF_EVT_FUNC_ENTER: {
                            auto data = reinterpret_cast<HPerfEventFuncEnter *>(event->data);
                            fprintf(
                                f,
                                R"({"type": "func-enter", "timestamp": {"sec": %llu, "nsec": %llu}, "func-name": "%s"})",
                                event->timestamp.sec,
                                event->timestamp.nsec,
                                data->func_name);
                            break;
                        }
                        case HPerfEventType::HPERF_EVT_FUNC_LEAVE: {
                            auto data = reinterpret_cast<HPerfEventFuncLeave *>(event->data);
                            fprintf(
                                f,
                                R"({"type": "func-leave", "timestamp": {"sec": %llu, "nsec": %llu}, "func-name": "%s"})",
                                event->timestamp.sec,
                                event->timestamp.nsec,
                                data->func_name);
                            break;
                        }
                        case HPerfEventType::HPERF_EVT_MEM_ALLOC: {
                            auto data = reinterpret_cast<HPerfEventMemAlloc *>(event->data);
                            fprintf(
                                f,
                                R"({"type": "mem-alloc", "timestamp": {"sec": %llu, "nsec": %llu}, "ptr": %zu, "size": %lu})",
                                event->timestamp.sec,
                                event->timestamp.nsec,
                                data->ptr,
                                data->size);
                            break;
                        }
                        case HPerfEventType::HPERF_EVT_MEM_FREE: {
                            auto data = reinterpret_cast<HPerfEventMemFree *>(event->data);
                            fprintf(f,
                                    R"({"type": "mem-free", "timestamp": {"sec": %llu, "nsec": %llu}, "ptr": %zu})",
                                    event->timestamp.sec,
                                    event->timestamp.nsec,
                                    data->ptr);
                            break;
                        }
                    }
                }
            }
        }
        fprintf(f, "]}\n");
        fclose(f);
    }
}

HPerfContext *hperf_context_get(size_t unique_id) {
    HPerfContext *ctx = hperf_contexts;
    while (ctx != nullptr) {
        if (ctx->unique_id == unique_id) {
            return ctx;
        }
        ctx = ctx->next;
    }
    return nullptr;
}

void hperf_init(const char *filename) {
    hperf_enabled = true;
    strcpy(hperf_report_filename, filename);
    if (hperf_contexts == nullptr) {
        hperf_contexts = hperf_context_create(runtime_get_thread_id());
    }
}
