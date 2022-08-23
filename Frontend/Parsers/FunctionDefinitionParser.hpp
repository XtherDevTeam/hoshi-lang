//
// Created by p0010 on 22-8-23.
//

#ifndef HOSHI_LANG_FUNCTIONDEFINITIONPARSER_HPP
#define HOSHI_LANG_FUNCTIONDEFINITIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class FunctionDefinitionParser : ParserBase {
        public:
            explicit FunctionDefinitionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_FUNCTIONDEFINITIONPARSER_HPP
