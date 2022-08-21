//
// Created by Jerry Chou on 2022/5/14.
//

#include "BytecodeInterpretError.hpp"

namespace Hoshi {
    BytecodeInterpretError::BytecodeInterpretError(const XString &String) : HoshiException(String) {

    }

    const char *BytecodeInterpretError::what() const noexcept {
        return HoshiException::what();
    }
} // Hoshi