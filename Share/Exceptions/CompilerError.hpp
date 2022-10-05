//
// Created by Jerry Chou on 2022/5/21.
//

#ifndef XSCRIPT2_COMPILERERROR_HPP
#define XSCRIPT2_COMPILERERROR_HPP

#include "HoshiException.hpp"

namespace Hoshi {

    class CompilerError : HoshiException {
    public:
        CompilerError(XIndexType Line, XIndexType Col, const XString &Text);

        [[nodiscard]] const char *what() const noexcept override;
    };

} // Hoshi

#endif //XSCRIPT2_COMPILERERROR_HPP
