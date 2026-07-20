#include "small_vector.h"

void *&SmallVector::operator[](unsigned index) {
    return data[index];
}

void SmallVector::clear() {
    size = 0;
}

void SmallVector::pop_back() {
    if (size > 0) {
        size--;
    }
    // No need to free the element, it's managed by the vector
}

void SmallVector::push_back(void *element) {
    if (size == capacity) {
        // Resize the vector if it's full
        capacity *= 2;
        data = (void **)mi_realloc(data, capacity * sizeof(void *));
    } else {
        // Add the element to the vector
        data[size++] = element;
    }
}

SmallVector::SmallVector(unsigned capacity) : capacity(capacity), size(0), data((void **)mi_malloc(capacity * sizeof(void *))) {}

void SmallVector::finalize() {
    mi_free(data);
}
