//
// Created by Jerry Chou on 22-8-22.
//

#ifndef HOSHI_LANG_STATICMEMBERACCESSEXPRESSIONPARSER_HPP
#define HOSHI_LANG_STATICMEMBERACCESSEXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class StaticMemberAccessExpressionParser : ParserBase {
        public:
            explicit StaticMemberAccessExpressionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_STATICMEMBERACCESSEXPRESSIONPARSER_HPP
