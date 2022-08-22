//
// Created by Jerry Chou on 22-8-22.
//

#ifndef HOSHI_LANG_PRIMARYPARSER_HPP
#define HOSHI_LANG_PRIMARYPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class PrimaryParser : ParserBase {
        public:
            explicit PrimaryParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_PRIMARYPARSER_HPP
