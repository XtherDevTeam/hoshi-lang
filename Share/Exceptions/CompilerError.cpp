//
// Created by Jerry Chou on 2022/5/21.
//

#include "CompilerError.hpp"

namespace Hoshi {
    CompilerError::CompilerError(XIndexType Line, XIndexType Col, const XString &Text) : HoshiException(
            L"[CompilerError] At line " + std::to_wstring(Line) + L" column " + std::to_wstring(Col) + L" : " + Text) {
    }

    const char *CompilerError::what() const noexcept {
        return HoshiException::what();
    }
} // Hoshi