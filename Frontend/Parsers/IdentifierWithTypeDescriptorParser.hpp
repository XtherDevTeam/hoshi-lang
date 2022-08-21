//
// Created by p0010 on 22-8-21.
//

#ifndef HOSHI_LANG_IDENTIFIERWITHTYPEDESCRIPTORPARSER_HPP
#define HOSHI_LANG_IDENTIFIERWITHTYPEDESCRIPTORPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class IdentifierWithTypeDescriptorParser : ParserBase {
        public:
            explicit IdentifierWithTypeDescriptorParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_IDENTIFIERWITHTYPEDESCRIPTORPARSER_HPP
