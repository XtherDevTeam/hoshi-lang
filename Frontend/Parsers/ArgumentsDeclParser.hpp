//
// Created by p0010 on 22-8-21.
//

#ifndef HOSHI_LANG_ARGUMENTSDECLPARSER_HPP
#define HOSHI_LANG_ARGUMENTSDECLPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class ArgumentsDeclParser : ParserBase {
            explicit ArgumentsDeclParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_ARGUMENTSDECLPARSER_HPP
