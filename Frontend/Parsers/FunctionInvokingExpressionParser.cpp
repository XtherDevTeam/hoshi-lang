//
// Created by p0010 on 22-8-22.
//

#include "FunctionInvokingExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        FunctionInvokingExpression::FunctionInvokingExpression(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST FunctionInvokingExpression::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser