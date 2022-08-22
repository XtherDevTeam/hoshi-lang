//
// Created by p0010 on 22-8-22.
//

#include "StaticMemberAccessExpressionWithTemplateArgumentsParser.hpp"
#include "IdentifierParser.hpp"
#include "TemplateArgumentsParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        StaticMemberAccessExpressionWithTemplateArgumentsParser::StaticMemberAccessExpressionWithTemplateArgumentsParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST StaticMemberAccessExpressionWithTemplateArgumentsParser::Parse() {
            AST Identifier = StaticMemberAccessExpressionParser(L).Parse();
            if (Identifier.IsNotMatchNode()) {
                return {};
            }
            AST TemplateArguments = TemplateArgumentsParser(L).Parse();
            if (TemplateArguments.IsNotMatchNode()) {
                return Identifier;
            }
            return {AST::TreeType::IdentifierWithTemplateArguments, {Identifier, TemplateArguments}};
        }
    } // Hoshi
} // Parser