//
// Created by p0010 on 22-8-22.
//

#include "AsExpressionParser.hpp"
#include "BooleanExpressionParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        AsExpressionParser::AsExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST AsExpressionParser::Parse() {
            AST LHS = BooleanExpressionParser(L).Parse();
            if (LHS.IsNotMatchNode()) {
                return {};
            }
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"as", 0, 0}) {
                return LHS;
            }
            AST RHS = StaticMemberAccessExpressionParser(L).Parse();
            return {AST::TreeType::AsExpression, {LHS, RHS}};
        }
    } // Hoshi
} // Parser