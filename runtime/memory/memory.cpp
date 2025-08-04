//
// Created by XIaokang00010 on 2025/7/29.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <runtime/build_config.h>
#include "memory.h"


extern "C" void *runtime_object_alloc(unsigned long size_in_bytes) { 
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Allocating %ld bytes of memory.\n", size_in_bytes);
    #endif
    void * ptr = calloc(size_in_bytes, 1);
    return ptr;
}
extern "C" void runtime_finalize_object(void *object) { 
    #if defined(ELYSIA_RUNTIME_BUILD_TYPE_DEBUG)
    printf("[Elysia/DEBUG] Finalizing object at %p.\n", object);
    #endif
    free(object);
}