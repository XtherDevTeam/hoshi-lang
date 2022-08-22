//
// Created by p0010 on 22-8-22.
//

#include "ArgumentsParser.hpp"

namespace Hoshi {
    namespace Parser {
        ArgumentsParser::ArgumentsParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST ArgumentsParser::Parse() {
            return Hoshi::AST();
        }
    } // Hoshi
} // Parser