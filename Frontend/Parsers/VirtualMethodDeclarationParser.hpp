//
// Created by p0010 on 22-8-26.
//

#ifndef HOSHI_LANG_VIRTUALMETHODDECLARATIONPARSER_HPP
#define HOSHI_LANG_VIRTUALMETHODDECLARATIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class VirtualMethodDeclarationParser : ParserBase {
        public:
            explicit VirtualMethodDeclarationParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_VIRTUALMETHODDECLARATIONPARSER_HPP
