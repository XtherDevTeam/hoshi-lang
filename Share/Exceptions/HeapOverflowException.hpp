//
// Created by Jerry Chou on 2022/5/14.
//

#ifndef XSCRIPT2_HEAPOVERFLOWEXCEPTION_HPP
#define XSCRIPT2_HEAPOVERFLOWEXCEPTION_HPP

#include "../HoshiException.hpp"

namespace Hoshi {

    class HeapOverflowException : HoshiException {
    public:
        explicit HeapOverflowException(const XString &String);

        [[nodiscard]] const char *what() const noexcept override;
    };

} // Hoshi

#endif //XSCRIPT2_HEAPOVERFLOWEXCEPTION_HPP
