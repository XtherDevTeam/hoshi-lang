//
// Created by Jerry Chou on 2022/5/14.
//

#ifndef XSCRIPT2_BYTECODEINTERPRETERROR_HPP
#define XSCRIPT2_BYTECODEINTERPRETERROR_HPP

#include "../HoshiException.hpp"

namespace Hoshi {

    class BytecodeInterpretError : HoshiException {
    public:
        explicit BytecodeInterpretError(const XString &String);

        [[nodiscard]] const char *what() const noexcept override;
    };

} // Hoshi

#endif //XSCRIPT2_BYTECODEINTERPRETERROR_HPP
