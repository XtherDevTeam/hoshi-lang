//
// Created by p0010 on 22-8-22.
//

#ifndef HOSHI_LANG_TYPEDESCRIPTOR_HPP
#define HOSHI_LANG_TYPEDESCRIPTOR_HPP

#include "ParserBase.hpp"

namespace Hoshi {
    namespace Parser {

        class TypeDescriptorParser : ParserBase {
        public:
            explicit TypeDescriptorParser(Hoshi::Lexer &L);

            Hoshi::AST Parse() override;
        };

    } // Hoshi
} // Parser

#endif //HOSHI_LANG_TYPEDESCRIPTOR_HPP
