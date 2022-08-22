//
// Created by p0010 on 22-8-22.
//

#include "AdditionExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        AdditionExpressionParser::AdditionExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST AdditionExpressionParser::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser