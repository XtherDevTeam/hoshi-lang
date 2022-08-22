//
// Created by Jerry Chou on 22-8-22.
//

#ifndef HOSHI_LANG_FUNCTIONINVOKEEXPRESSIONPARSER_HPP
#define HOSHI_LANG_FUNCTIONINVOKEEXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class FunctionInvokeExpressionParser : ParserBase {
        public:
            explicit FunctionInvokeExpressionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_FUNCTIONINVOKEEXPRESSIONPARSER_HPP
