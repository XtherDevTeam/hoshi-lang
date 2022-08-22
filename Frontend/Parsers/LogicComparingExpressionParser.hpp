//
// Created by p0010 on 22-8-22.
//

#ifndef HOSHI_LANG_LOGICCOMPARINGEXPRESSIONPARSER_HPP
#define HOSHI_LANG_LOGICCOMPARINGEXPRESSIONPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class LogicComparingExpressionParser : ParserBase {
        public:
            explicit LogicComparingExpressionParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_LOGICCOMPARINGEXPRESSIONPARSER_HPP
