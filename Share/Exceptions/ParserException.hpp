//
// Created by Jerry Chou on 2022/5/21.
//

#ifndef XSCRIPT2_PARSEREXCEPTION_HPP
#define XSCRIPT2_PARSEREXCEPTION_HPP

#include "Utils.hpp"
#include "HoshiException.hpp"

namespace Hoshi {

    class ParserException : HoshiException {
        std::string message;
    public:
        ParserException();

        ParserException(XIndexType Line, XIndexType Column, const XString &Reason);

        [[nodiscard]] const char *what() const noexcept override;
    };

} // Hoshi

#endif //XSCRIPT2_PARSEREXCEPTION_HPP
