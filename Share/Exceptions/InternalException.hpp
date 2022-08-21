//
// Created by Jerry Chou on 2022/5/21.
//

#ifndef XSCRIPT2_INTERNALEXCEPTION_HPP
#define XSCRIPT2_INTERNALEXCEPTION_HPP

#include "../HoshiException.hpp"

namespace Hoshi {

    class InternalException : HoshiException {
    public:
        explicit InternalException(const XString &Str);

        [[nodiscard]] const char *what() const noexcept override;
    };

} // Hoshi

#endif //XSCRIPT2_INTERNALEXCEPTION_HPP
