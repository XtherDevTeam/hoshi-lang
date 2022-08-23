//
// Created by p0010 on 22-8-23.
//

#include "IdentifierWithTypeDescriptorParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        IdentifierWithTypeDescriptorParser::IdentifierWithTypeDescriptorParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST IdentifierWithTypeDescriptorParser::Parse() {
            AST LHS;
            if (L.LastToken.Kind == Lexer::TokenKind::Identifier) {
                LHS = {AST::TreeType::Identifier, L.LastToken};
                L.Scan();
            } else {
                return {};
            }
            if (L.LastToken.Kind == Lexer::TokenKind::TypeDescriptorSign) {
                L.Scan();
            } else {
                return {};
            }
            AST RHS = StaticMemberAccessExpressionParser(L).Parse();
            if (RHS.IsNotMatchNode()) {
                Throw(L"IdentifierWithTypeDescriptorParser", L"Expected a typename here");
                return {};
            }
            return {AST::TreeType::IdentifierWithTypeDescriptor, {LHS, RHS}};
        }
    } // Hoshi
} // Parser