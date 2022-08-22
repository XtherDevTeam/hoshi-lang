//
// Created by Jerry Chou on 22-8-22.
//

#ifndef HOSHI_LANG_SINGLEEXPRESSIONPARSER_HPP
#define HOSHI_LANG_SINGLEEXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class SingleExpressionParser : ParserBase {
        public:
            explicit SingleExpressionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_SINGLEEXPRESSIONPARSER_HPP
