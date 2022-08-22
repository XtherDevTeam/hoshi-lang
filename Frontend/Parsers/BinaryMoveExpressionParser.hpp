//
// Created by Jerry Chou on 22-8-22.
//

#ifndef HOSHI_LANG_BINARYMOVEEXPRESSIONPARSER_HPP
#define HOSHI_LANG_BINARYMOVEEXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class BinaryMoveExpressionParser : ParserBase {
        public:
            explicit BinaryMoveExpressionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_BINARYMOVEEXPRESSIONPARSER_HPP
