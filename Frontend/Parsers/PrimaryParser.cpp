//
// Created by Jerry Chou on 22-8-22.
//

#include "PrimaryParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"
#include "LiteralsParser.hpp"
#include "FunctionInvokeExpressionParser.hpp"
#include "AsExpressionParser.hpp"
#include "AssignmentExpressionParser.hpp"
#include "ExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        PrimaryParser::PrimaryParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST PrimaryParser::Parse() {
            if (L.LastToken.Kind == Lexer::TokenKind::LeftParentheses) {
                L.Scan();
                AST Expr = ExpressionParser(L).Parse();
                if (L.LastToken.Kind != Lexer::TokenKind::RightParentheses) {
                    Throw(L"PrimaryParser", L"Expected a `)`");
                    return {};
                }
                L.Scan();
                return Expr;
            }

            AST Tree = FunctionInvokeExpressionParser(L).Parse();
            if (!Tree.IsNotMatchNode())
                return Tree;
            Tree = LiteralsParser(L).Parse();
            if (!Tree.IsNotMatchNode())
                return Tree;

            return {};
        }
    } // Hoshi
} // Parser