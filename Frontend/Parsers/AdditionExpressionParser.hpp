//
// Created by Jerry Chou on 22-8-22.
//

#ifndef HOSHI_LANG_ADDITIONEXPRESSIONPARSER_HPP
#define HOSHI_LANG_ADDITIONEXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class AdditionExpressionParser : ParserBase {
        public:
            explicit AdditionExpressionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_ADDITIONEXPRESSIONPARSER_HPP
