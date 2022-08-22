//
// Created by p0010 on 22-8-22.
//

#include "MultiplicationExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        MultiplicationExpressionParser::MultiplicationExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST MultiplicationExpressionParser::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser