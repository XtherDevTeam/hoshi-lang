//
// Created by XIaokang00010 on 2026/7/20.
//

#include "bacon.h"

thread_local LocalBufferNode local_buffer;

BufferNode *global_buffer_head;

unsigned int local_thread_should_sleep;

int bacon_current_active_threads;

unsigned int current_global_buffer_page_count;

pthread_t bacon_thread_handle;

void bacon_flush_local_buffer() {
    if (local_buffer.count == 0)
        return;

    auto new_node = (BufferNode *)mi_malloc(sizeof(BufferNode));
    memcpy(new_node->entries, local_buffer.entries, sizeof(void *) * local_buffer.count);
    if (local_buffer.count < ELYSIA_BACON_LOCAL_BUFFER_SIZE) {
        new_node->entries[local_buffer.count] = nullptr;
    }
    local_buffer.count = 0;
    do {
        new_node->next = __atomic_load_n(&global_buffer_head, __ATOMIC_RELAXED);
    } while (!__atomic_compare_exchange_n(&global_buffer_head,
                                          &new_node->next,
                                          new_node,
                                          true,
                                          __ATOMIC_RELEASE, // 使用 RELEASE 保证 memcpy 的数据对 GC 线程可见
                                          __ATOMIC_RELAXED));
    __atomic_fetch_add(&current_global_buffer_page_count, 1, __ATOMIC_RELEASE); // atomic operation to increment page count
}
void bacon_flush_global_buffer() {
    // in STW mode, atomic operations are not needed
    while (global_buffer_head != nullptr) {
        auto next = global_buffer_head->next;
        mi_free(global_buffer_head);
        global_buffer_head = next;
    }

    current_global_buffer_page_count = 0; // reset page count
}
void bacon_push_to_local_buffer(YoiObject *object) {
    if (object->bacon_mark.is_buffered())
        return;

    if (__atomic_sub_fetch(&object->gc_refcount, 1, __ATOMIC_RELEASE) == 0) {
        auto &rtti_query = rtti_table[object->type_id];
        if (rtti_query.finalizer) {
            rtti_query.finalizer(object);
        }
        runtime_finalize_object(object);
    } else {
        if (object->bacon_mark.try_mark_candidate()) {

            // not zero after decrementing
            if (local_buffer.count < ELYSIA_BACON_LOCAL_BUFFER_SIZE) {
                local_buffer.entries[local_buffer.count++] = object;
            } else {
                bacon_flush_local_buffer();
                local_buffer.entries[local_buffer.count++] = object;
            }
        }
    }
}
void bacon_poll() {
    if (UNLIKELY(__atomic_load_n(&local_thread_should_sleep, __ATOMIC_RELAXED) == 1)) {
        bacon_flush_local_buffer();

        __atomic_fetch_sub(&bacon_current_active_threads, 1, __ATOMIC_RELEASE);
        unsigned int expected = 1;

        while (__atomic_load_n(&local_thread_should_sleep, __ATOMIC_RELAXED) == 1) {
            wait_on_address(&local_thread_should_sleep, expected);
        }

        __atomic_fetch_add(&bacon_current_active_threads, 1, __ATOMIC_RELEASE);
    }
}
void bacon_mark_grey(YoiObject *object) {
    if (object->bacon_mark.get_color() != BaconMark::Color::Attempted) {

        object->bacon_mark.set_color(BaconMark::Color::Attempted);
        runtime_trace_yoi_object(
            object,
            [](YoiObject *child, void *pUserData) {
                child->gc_refcount--; // simulate decrementing refcount
                bacon_mark_grey(child);
            },
            nullptr);
    }
}
void bacon_scan_black(YoiObject *obj) {
    obj->bacon_mark.set_color(BaconMark::Color::Survive);
    runtime_trace_yoi_object(
        obj,
        [](YoiObject *child, void *pUserData) {
            child->gc_refcount++; // reset refcount
            if (child->bacon_mark.get_color() != BaconMark::Color::Survive) {
                bacon_scan_black(child);
            }
        },
        nullptr);
}
void bacon_scan(YoiObject *obj) {
    if (obj->bacon_mark.get_color() == BaconMark::Color::Attempted) {
        if (obj->gc_refcount > 0) {
            bacon_scan_black(obj); // strong reference
        } else {
            obj->bacon_mark.set_color(BaconMark::Color::Garbage); // confirmed isolated
            runtime_trace_yoi_object(
                obj, [](YoiObject *child, void *pUserData) { bacon_scan(child); }, nullptr);
        }
    }
}
void bacon_collect_white(SmallVector &result, YoiObject *obj) {
    if (obj->bacon_mark.get_color() == BaconMark::Color::Garbage) {
        obj->bacon_mark.set_color(BaconMark::Color::Survive); // avoid double free

        runtime_trace_yoi_object(
            obj,
            [](YoiObject *child, void *pResult) {
                SmallVector &result = *(SmallVector *)pResult; // cast void* to SmallVector* and get reference
                bacon_collect_white(result, child);
            },
            &result);

        result.push_back((void *)obj); // collect white
    }
}
void bacon_stw() {
    // stop-the-world
    __atomic_store_n(&local_thread_should_sleep, 1, __ATOMIC_RELEASE);
    while (__atomic_load_n(&bacon_current_active_threads, __ATOMIC_ACQUIRE) > 0)
        CPU_RELAX();

    // collect garbage
    // mark grey
    for (BufferNode *node = global_buffer_head; node != nullptr; node = node->next) {
        for (int i = 0; i < ELYSIA_BACON_LOCAL_BUFFER_SIZE && node->entries[i]; i++) {
            if (node->entries[i]->bacon_mark.get_color() == BaconMark::Color::Candidate)
                bacon_mark_grey(node->entries[i]);
        }
    }

    // scan again, find garbage
    for (BufferNode *node = global_buffer_head; node != nullptr; node = node->next) {
        for (int i = 0; i < ELYSIA_BACON_LOCAL_BUFFER_SIZE && node->entries[i]; i++) {
            bacon_scan(node->entries[i]);
        }
    }

    // collect white
    SmallVector collected(1024);

    for (BufferNode *node = global_buffer_head; node != nullptr; node = node->next) {
        for (int i = 0; i < ELYSIA_BACON_LOCAL_BUFFER_SIZE && node->entries[i]; i++) {
            node->entries[i]->bacon_mark.set_buffered(false);
            bacon_collect_white(collected, node->entries[i]);
        }
    }

    bacon_flush_global_buffer();

    // wake up threads
    __atomic_store_n(&local_thread_should_sleep, 0, __ATOMIC_RELEASE);
    wake_all_on_address(&local_thread_should_sleep);

    // now proceed finalization
    for (int i = 0; i < collected.size; i++) {
        auto &rtti_query = rtti_table[((YoiObject *)collected[i])->type_id];
        if (rtti_query.finalizer) {
            rtti_query.finalizer(collected[i]);
        }
        runtime_finalize_object((YoiObject *)collected[i]);
    }

    collected.finalize();
}

void *bacon_recycler_thread(void *args) {
    while (true) {
        while (__atomic_load_n(&current_global_buffer_page_count, __ATOMIC_ACQUIRE) < ELYSIA_BACON_MAX_GLOBAL_BUFFER_PAGE_COUNT)
            CPU_RELAX();
        bacon_stw();
        // it automatically flushes global buffer after stw
    }
    return nullptr;
}
void bacon_init() {
    global_buffer_head = nullptr;
    local_thread_should_sleep = 0;
    bacon_current_active_threads = 0;

    if (pthread_create(&bacon_thread_handle, nullptr, &bacon_recycler_thread, nullptr)) {
        fprintf(stderr, "[Elysia/ERROR] hoshi-lang runtime: failed to create bacon thread\n");
        exit(1);
    }
}
