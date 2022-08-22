//
// Created by p0010 on 22-8-22.
//

#ifndef HOSHI_LANG_BINARYEXPRESSIONPARSER_HPP
#define HOSHI_LANG_BINARYEXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class BinaryExpressionParser : ParserBase {
        public:
            explicit BinaryExpressionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_BINARYEXPRESSIONPARSER_HPP
