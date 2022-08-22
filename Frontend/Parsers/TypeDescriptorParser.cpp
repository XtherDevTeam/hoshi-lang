//
// Created by p0010 on 22-8-22.
//

#include "TypeDescriptorParser.hpp"
#include "StaticMemberAccessExpressionWithTemplateArgumentsParser.hpp"

namespace Hoshi {
    namespace Parser {
        TypeDescriptorParser::TypeDescriptorParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST TypeDescriptorParser::Parse() {
            AST Tree = StaticMemberAccessExpressionWithTemplateArgumentsParser(L).Parse();
            return Tree;
        }
    } // Hoshi
} // Parser