//
// Created by XIaokang00010 on 2025/6/29.
//

#ifndef HOSHI_LANG_MEMORY_H
#define HOSHI_LANG_MEMORY_H

extern "C" void *runtime_object_alloc(long size_in_bytes);

extern "C" void runtime_finalize_object(void *object);

#endif //HOSHI_LANG_MEMORY_H