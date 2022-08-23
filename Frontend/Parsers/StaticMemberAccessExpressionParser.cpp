//
// Created by Jerry Chou on 22-8-22.
//

#include "StaticMemberAccessExpressionParser.hpp"
#include "MemberExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        StaticMemberAccessExpressionParser::StaticMemberAccessExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST StaticMemberAccessExpressionParser::Parse() {
            AST Tree = MemberExpressionParser(L).Parse(), Ret = Tree;
            if (Tree.IsNotMatchNode())
                return {};
            if (Tree.Type == AST::TreeType::MemberExpression || L.LastToken.Kind != Lexer::TokenKind::StaticMemberAccessSign)
                return Tree;

            L.Scan();

            Tree = MemberExpressionParser(L).Parse();
            while (!Tree.IsNotMatchNode() and Tree.Type != AST::TreeType::MemberExpression) {
                if (L.LastToken.Kind == Lexer::TokenKind::StaticMemberAccessSign) {
                    L.Scan();
                    if (!Ret.IsNotMatchNode())
                        Ret = {AST::TreeType::StaticMemberAccessExpression, {Ret, Tree}};
                    Tree = MemberExpressionParser(L).Parse();
                } else {
                    Ret = {AST::TreeType::StaticMemberAccessExpression, {Ret, Tree}};
                    return Ret;
                }
            }
            Ret = {AST::TreeType::StaticMemberAccessExpression, {Ret, Tree}};
            return Ret;
        }
    } // Hoshi
} // Parser