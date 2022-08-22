//
// Created by Jerry Chou on 22-8-22.
//

#ifndef HOSHI_LANG_BOOLEANEXPRESSIONPARSER_HPP
#define HOSHI_LANG_BOOLEANEXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class BooleanExpressionParser : ParserBase {
        public:
            explicit BooleanExpressionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_BOOLEANEXPRESSIONPARSER_HPP
