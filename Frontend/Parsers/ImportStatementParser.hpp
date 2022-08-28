//
// Created by p0010 on 22-8-27.
//

#ifndef HOSHI_LANG_IMPORTSTATEMENTPARSER_HPP
#define HOSHI_LANG_IMPORTSTATEMENTPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class ImportStatementParser : ParserBase {
        public:
            explicit ImportStatementParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_IMPORTSTATEMENTPARSER_HPP
