//
// Created by p0010 on 22-8-22.
//

#include "LogicEqualExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        LogicEqualExpressionParser::LogicEqualExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST LogicEqualExpressionParser::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser