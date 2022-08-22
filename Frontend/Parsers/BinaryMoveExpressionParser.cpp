//
// Created by p0010 on 22-8-22.
//

#include "BinaryMoveExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        BinaryMoveExpressionParser::BinaryMoveExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST BinaryMoveExpressionParser::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser