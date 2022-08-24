//
// Created by p0010 on 22-8-24.
//

#ifndef HOSHI_LANG_SIMPLESTATEMENTPARSER_HPP
#define HOSHI_LANG_SIMPLESTATEMENTPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class SimpleStatementParser : ParserBase {
        public:
            explicit SimpleStatementParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_SIMPLESTATEMENTPARSER_HPP
