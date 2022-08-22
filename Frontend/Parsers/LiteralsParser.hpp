//
// Created by Jerry Chou on 22-8-22.
//

#ifndef HOSHI_LANG_LITERALSPARSER_HPP
#define HOSHI_LANG_LITERALSPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class LiteralsParser : ParserBase {
        public:
            explicit LiteralsParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_LITERALSPARSER_HPP
