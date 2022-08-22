//
// Created by p0010 on 22-8-22.
//

#ifndef HOSHI_LANG_FUNCTIONINVOKINGEXPRESSION_HPP
#define HOSHI_LANG_FUNCTIONINVOKINGEXPRESSION_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class FunctionInvokingExpression : ParserBase {
        public:
            explicit FunctionInvokingExpression(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_FUNCTIONINVOKINGEXPRESSION_HPP
