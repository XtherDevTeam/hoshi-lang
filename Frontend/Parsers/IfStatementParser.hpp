//
// Created by p0010 on 22-8-24.
//

#ifndef HOSHI_LANG_IFSTATEMENTPARSER_HPP
#define HOSHI_LANG_IFSTATEMENTPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class IfStatementParser : ParserBase {
        public:
            explicit IfStatementParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_IFSTATEMENTPARSER_HPP
