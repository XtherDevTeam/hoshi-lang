//
// Created by p0010 on 22-8-22.
//

#include "StaticMemberAccessExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        StaticMemberAccessExpressionParser::StaticMemberAccessExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST StaticMemberAccessExpressionParser::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser