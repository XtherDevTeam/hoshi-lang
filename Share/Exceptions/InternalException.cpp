//
// Created by Jerry Chou on 2022/5/21.
//

#include "InternalException.hpp"

namespace Hoshi {
    InternalException::InternalException(const XString &Str) : HoshiException(Str) {

    }

    const char *InternalException::what() const noexcept {
        return HoshiException::what();
    }
} // Hoshi