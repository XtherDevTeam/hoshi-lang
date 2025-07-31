//
// Created by XIaokang00010 on 2025/7/29.
//

#include <cstdio>
#include <cstdlib>

extern "C" void *runtime_object_alloc(unsigned long size_in_bytes) { 
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Allocating %ld bytes of memory.\n", size_in_bytes);
    #endif
    return malloc(size_in_bytes);
}
extern "C" void runtime_finalize_object(void *object) { 
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Finalizing object at %p.\n", object);
    #endif
    free(object);
}