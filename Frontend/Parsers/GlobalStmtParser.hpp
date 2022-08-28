//
// Created by p0010 on 22-8-27.
//

#ifndef HOSHI_LANG_GLOBALSTMTPARSER_HPP
#define HOSHI_LANG_GLOBALSTMTPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class GlobalStmtParser : ParserBase {
        public:
            explicit GlobalStmtParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_GLOBALSTMTPARSER_HPP
