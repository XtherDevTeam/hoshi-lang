//
// Created by p0010 on 22-8-23.
//

#ifndef HOSHI_LANG_VARIABLEDECLARATIONPARSER_HPP
#define HOSHI_LANG_VARIABLEDECLARATIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class VariableDeclarationParser : ParserBase {
        public:
            explicit VariableDeclarationParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_VARIABLEDECLARATIONPARSER_HPP
