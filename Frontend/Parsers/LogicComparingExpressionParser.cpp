//
// Created by p0010 on 22-8-22.
//

#include "LogicComparingExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        LogicComparingExpressionParser::LogicComparingExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST LogicComparingExpressionParser::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser