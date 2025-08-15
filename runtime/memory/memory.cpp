//
// Created by XIaokang00010 on 2025/7/29.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <runtime/build_config.h>
#include "memory.h"
#include "runtime/rtti/rtti.h"

#if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
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

extern "C" int64_t runtime_object_allocated = 0;

extern "C" void *runtime_object_alloc_report(size_t size, void *object) { 
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Allocating %lld bytes memory at %p. Current object count: %lld.\n", size, object, runtime_object_allocated);
    #endif
    runtime_object_allocated ++;
    #ifdef ELYSIA_RUNTIME_BUILD_TYPE_DEBUG
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
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Finalizing %s object at %p. Current object count: %lld.\n", rtti_table[object->type_id].type_name, object, runtime_object_allocated);
    #endif
    runtime_object_allocated --;
    #ifdef ELYSIA_RUNTIME_BUILD_TYPE_DEBUG
    for (AllocatedMemoryList *node = allocated_memory_list; node!= nullptr; node = node->next) {
        if (node->memory == object) {
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
    free(ptr);
}

void *runtime_object_alloc(unsigned long size) {
    void *ptr = malloc(size);
    runtime_object_alloc_report(size, ptr);
    return ptr;
}
