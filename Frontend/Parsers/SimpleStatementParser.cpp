//
// Created by p0010 on 22-8-24.
//

#include "SimpleStatementParser.hpp"

namespace Hoshi {
    namespace Parser {
        SimpleStatementParser::SimpleStatementParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST SimpleStatementParser::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser