//
// Created by p0010 on 22-8-25.
//

#ifndef HOSHI_LANG_INNERCLASSDEFINITIONSTMTPARSER_HPP
#define HOSHI_LANG_INNERCLASSDEFINITIONSTMTPARSER_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class InnerClassDefinitionStmtParser : ParserBase {
        public:
            explicit InnerClassDefinitionStmtParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_INNERCLASSDEFINITIONSTMTPARSER_HPP
