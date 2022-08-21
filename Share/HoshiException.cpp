//
// Created by Jerry Chou on 2022/5/7.
//

#include "HoshiException.hpp"

namespace Hoshi {
    HoshiException::HoshiException(const XString &String) : String(wstring2string(String)) {

    }

    const char *HoshiException::what() const noexcept {
        return String.c_str();
    }

    HoshiException::HoshiException() = default;
} // Hoshi