//
// Created by Jerry Chou on 22-8-22.
//

#ifndef HOSHI_LANG_LOGICEQUALEXPRESSIONPARSER_HPP
#define HOSHI_LANG_LOGICEQUALEXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class LogicEqualExpressionParser : ParserBase {
        public:
            explicit LogicEqualExpressionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_LOGICEQUALEXPRESSIONPARSER_HPP
