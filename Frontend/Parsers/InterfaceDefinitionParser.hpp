//
// Created by p0010 on 22-8-26.
//

#ifndef HOSHI_LANG_INTERFACEDEFINITIONPARSER_HPP
#define HOSHI_LANG_INTERFACEDEFINITIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class InterfaceDefinitionParser : ParserBase {
        public:
            explicit InterfaceDefinitionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_INTERFACEDEFINITIONPARSER_HPP
