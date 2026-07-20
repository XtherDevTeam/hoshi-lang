//
// Created by XIaokang00010 on 2026/7/20.
//

#ifndef HOSHI_RUNTIME_BACON_SMALL_VECTOR_H
#define HOSHI_RUNTIME_BACON_SMALL_VECTOR_H

#include <mimalloc/include/mimalloc.h>

struct SmallVector {
    unsigned capacity;
    unsigned size;
    void** data;

    SmallVector(unsigned capacity);

    void push_back(void *element);

    void pop_back();

    void clear();

    void *&operator[](unsigned index);

    void finalize();
};

#endif // HOSHI_RUNTIME_BACON_SMALL_VECTOR_H