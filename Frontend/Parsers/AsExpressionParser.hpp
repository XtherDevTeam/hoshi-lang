
//
// Created by p0010 on 22-8-22.
//

#ifndef HOSHI_LANG_ASEXPRESSIONPARSER_HPP
#define HOSHI_LANG_ASEXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class AsExpressionParser : ParserBase {
        public:
            explicit AsExpressionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_ASEXPRESSIONPARSER_HPP
