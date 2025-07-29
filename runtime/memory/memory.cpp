//
// Created by XIaokang00010 on 2025/6/29.
//

#include <cstdio>
#include <cstdlib>

void *runtime_object_alloc(long size_in_bytes) { 
    printf("[Elysia/DEBUG] Allocating %ld bytes of memory.\n", size_in_bytes);
    return malloc(size_in_bytes);
}
void runtime_finalize_object(void *object) { 
    printf("[Elysia/DEBUG] Finalizing object at %p.\n", object);
    free(object);
}