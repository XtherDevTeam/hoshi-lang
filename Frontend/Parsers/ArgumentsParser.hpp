//
// Created by p0010 on 22-8-22.
//

#ifndef HOSHI_LANG_ARGUMENTSPARSER_HPP
#define HOSHI_LANG_ARGUMENTSPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class ArgumentsParser : ParserBase {
        public:
            explicit ArgumentsParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_ARGUMENTSPARSER_HPP
