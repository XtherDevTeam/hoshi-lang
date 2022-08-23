//
// Created by p0010 on 22-8-23.
//

#ifndef HOSHI_LANG_ARGUMENTSDECLARATIONPARSER_HPP
#define HOSHI_LANG_ARGUMENTSDECLARATIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class ArgumentsDeclarationParser : ParserBase    {
        public:
            explicit ArgumentsDeclarationParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_ARGUMENTSDECLARATIONPARSER_HPP
