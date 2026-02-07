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

#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
    #include <sys/mman.h>
    #include <unistd.h>
#endif

#ifndef ELYSIA_DISABLE_MEMORY_EXECUTABLE_MAPPING_FEATURE

void *runtime_exec_permit_alloc(unsigned long size) {
    #ifdef _WIN32
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    #elif defined(__linux__) || defined(__APPLE__)
    // Align to 16 bytes for safe cross-platform JIT structure (metadata + alignment padding)
    size_t header_size = 16;
    size_t real_size = size + header_size;
    
    int prot = PROT_READ | PROT_WRITE | PROT_EXEC;
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
    
    #if defined(__APPLE__) && defined(__aarch64__)
    flags |= MAP_JIT;
    #endif

    void *ptr = mmap(NULL, real_size, prot, flags, -1, 0);
    
    if (ptr == MAP_FAILED) {
        return nullptr;
    }
    
    // Store the allocated size at the beginning
    *reinterpret_cast<size_t*>(ptr) = real_size;
    
    // Return pointer offset by header_size
    return static_cast<char*>(ptr) + header_size;
    #else
    return nullptr;
    #endif
}

void runtime_exec_permit_free(void *ptr) {
    if (!ptr) return;
    
    #ifdef _WIN32
    VirtualFree(ptr, 0, MEM_RELEASE);
    #elif defined(__linux__) || defined(__APPLE__)
    size_t header_size = 16;
    char *real_ptr = static_cast<char*>(ptr) - header_size;
    size_t real_size = *reinterpret_cast<size_t*>(real_ptr);
    
    munmap(real_ptr, real_size);
    #endif
}

#endif
