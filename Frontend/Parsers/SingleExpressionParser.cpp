//
// Created by p0010 on 22-8-22.
//

#include "SingleExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        SingleExpressionParser::SingleExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST SingleExpressionParser::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser