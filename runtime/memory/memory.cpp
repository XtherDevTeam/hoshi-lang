//
// Created by XIaokang00010 on 2025/7/29.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <mimalloc/include/mimalloc.h>

#include <runtime/build_config.h>
#include "memory.h"
#include "runtime/rtti/rtti.h"

#if defined(ELYSIA_RUNTIME_HPERF_ENABLE)
#include <runtime/hperf/hperf.h>
#endif

#if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG) && defined(ELYSIA_RUNTIME_ENABLE_BUILTIN_MEMORY_LEAK_DETECTOR)
extern "C" AllocatedMemoryList *allocated_memory_list = nullptr;

void runtime_debug_print_current_allocated_memory() {
    for (AllocatedMemoryList *node = allocated_memory_list; node; node = node->next) {
        printf("[Elysia/DEBUG] | Memory of %s at %p, size: %ld bytes. target refcount: %lld.\n",
               rtti_table[((YoiObject *)node->memory)->type_id].type_name,
               node->memory,
               node->size,
               ((YoiObject *)node->memory)->gc_refcount);
    }
}
#endif

#if defined(ELYSIA_RUNTIME_ENABLE_BUILTIN_MEMORY_LEAK_DETECTOR)
int64_t runtime_object_allocated = 0;
#endif

extern "C" void *runtime_object_alloc_report(size_t size, void *object) { 
    #if defined (ELYSIA_RUNTIME_HPERF_ENABLE) 
    hperf_report_mem_alloc(object, size);
    #endif
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG) && defined(ELYSIA_RUNTIME_ENABLE_BUILTIN_MEMORY_LEAK_DETECTOR)
    printf("[Elysia/DEBUG] Allocating %zu bytes memory at %p. Current object count: %lld.\n", size, object, runtime_object_allocated);
    runtime_object_allocated ++;
    #endif
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG) && defined(ELYSIA_RUNTIME_ENABLE_BUILTIN_MEMORY_LEAK_DETECTOR)
    if (allocated_memory_list == nullptr) {
        allocated_memory_list = static_cast<AllocatedMemoryList *>(malloc(sizeof(AllocatedMemoryList)));
        allocated_memory_list->memory = object;
        allocated_memory_list->size = size;
        allocated_memory_list->prev = nullptr;
        allocated_memory_list->next = nullptr;
    } else {
        auto *new_node = static_cast<AllocatedMemoryList *>(malloc(sizeof(AllocatedMemoryList)));
        new_node->memory = object;
        new_node->size = size;
        new_node->prev = nullptr;
        new_node->next = allocated_memory_list;
        allocated_memory_list->prev = new_node;
        allocated_memory_list = new_node;
    }
    #endif
    return object;
}

extern "C" void runtime_finalize_object_report(YoiObject *object) { 
    #if defined(ELYSIA_RUNTIME_HPERF_ENABLE)
    hperf_report_mem_free(object);
    #endif
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG) && defined(ELYSIA_RUNTIME_ENABLE_BUILTIN_MEMORY_LEAK_DETECTOR)
    printf("[Elysia/DEBUG] Finalizing %s object at %p. Current object count: %lld.\n", rtti_table[object->type_id].type_name, object, runtime_object_allocated);
    runtime_object_allocated --;

    for (AllocatedMemoryList *node = allocated_memory_list; node!= nullptr; node = node->next) {
        if (node->memory && node->memory == object) {
            if (node->prev != nullptr) {
                node->prev->next = node->next;
            } else {
                allocated_memory_list = node->next;
            }
            if (node->next != nullptr) {
                node->next->prev = node->prev;
            }
            free(node);
        }
    }
    runtime_debug_print_current_allocated_memory();
    #endif
}

GC_WRAPPER_IMPL(int, YoiIntegerObject);

GC_WRAPPER_IMPL(decimal, YoiDecimalObject);

GC_WRAPPER_IMPL(bool, YoiBooleanObject);

GC_WRAPPER_IMPL(char, YoiCharObject);

GC_WRAPPER_IMPL(string, YoiStringObject);

void runtime_finalize_object(YoiObject *object) {
    runtime_finalize_object_report(object);
    void *ptr = object;
    mi_free(ptr);
}

void *runtime_object_alloc(unsigned long size) {
    void *ptr = mi_calloc(size, 1);
    runtime_object_alloc_report(size, ptr);
    return ptr;
}

YoiIntegerObject *runtime_get_string_array_data_pointer(YoiObjectArray *array) {
    auto raw = reinterpret_cast<int64_t>(&array->data);
    auto *obj = (YoiIntegerObject *)runtime_object_alloc(sizeof(YoiIntegerObject));
    obj->gc_refcount = 1;
    obj->type_id = 0;
    obj->value = raw;
    
    if (--array->gc_refcount == 0)
        runtime_finalize_object((YoiObject*)array);
    return obj;
}
