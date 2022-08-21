//
// Created by Jerry Chou on 2022/5/14.
//

#include "HeapOverflowException.hpp"

namespace Hoshi {
    HeapOverflowException::HeapOverflowException(const XString &String) : HoshiException(
            L"HeapOverflowException: " + String) {

    }

    const char *HeapOverflowException::what() const noexcept {
        return HoshiException::what();
    }
} // Hoshi