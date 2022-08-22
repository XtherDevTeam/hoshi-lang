//
// Created by Jerry Chou on 22-8-22.
//

#include "IdentifierParser.hpp"
#include "TemplateArgumentsParser.hpp"

namespace Hoshi {
    namespace Parser {
        IdentifierParser::IdentifierParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST IdentifierParser::Parse() {
            if (L.LastToken.Kind != Lexer::TokenKind::Identifier) {
                Rollback();
                return {};
            }
            AST Res{AST::TreeType::Identifier, L.LastToken};
            L.Scan();
            AST TA = TemplateArgumentsParser(L).Parse();
            if (!TA.IsNotMatchNode())
                return {AST::TreeType::IdentifierWithTemplateArguments, {Res, TA}};
            return Res;
        }
    } // Hoshi
} // Parser