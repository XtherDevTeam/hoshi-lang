//
// Created by p0010 on 22-8-22.
//

#include "BinaryExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        BinaryExpressionParser::BinaryExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST BinaryExpressionParser::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser