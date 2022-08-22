//
// Created by Jerry Chou on 22-8-22.
//

#ifndef HOSHI_LANG_MULTIPLICATIONEXPRESSIONPARSER_HPP
#define HOSHI_LANG_MULTIPLICATIONEXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class MultiplicationExpressionParser : ParserBase {
        public:
            explicit MultiplicationExpressionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_MULTIPLICATIONEXPRESSIONPARSER_HPP
