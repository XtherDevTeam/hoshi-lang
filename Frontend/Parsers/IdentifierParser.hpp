//
// Created by p0010 on 22-8-21.
//

#ifndef HOSHI_LANG_IDENTIFIERPARSER_HPP
#define HOSHI_LANG_IDENTIFIERPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class IdentifierParser : ParserBase {
        public:
            explicit IdentifierParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_IDENTIFIERPARSER_HPP
