//
// Created by p0010 on 22-8-22.
//

#include "MemberExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        MemberExpressionParser::MemberExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST MemberExpressionParser::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser