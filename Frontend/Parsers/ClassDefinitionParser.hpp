//
// Created by p0010 on 22-8-25.
//

#ifndef HOSHI_LANG_CLASSDEFINITIONPARSER_HPP
#define HOSHI_LANG_CLASSDEFINITIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class ClassDefinitionParser : ParserBase {
        public:
            explicit ClassDefinitionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_CLASSDEFINITIONPARSER_HPP
