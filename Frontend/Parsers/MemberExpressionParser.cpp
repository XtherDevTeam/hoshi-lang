//
// Created by Jerry Chou on 22-8-22.
//

#include "MemberExpressionParser.hpp"
#include "IdentifierParser.hpp"
#include "FunctionInvokeExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        MemberExpressionParser::MemberExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST MemberExpressionParser::Parse() {
            AST Current = IdentifierParser(L).Parse();
            while (!Current.IsNotMatchNode()) {
                if (L.LastToken.Kind == Lexer::TokenKind::Dot) {
                    L.Scan();
                    Current = {AST::TreeType::MemberExpression, {Current, IdentifierParser(L).Parse()}};
                    if (Current.Subtrees[1].IsNotMatchNode()) {
                        Throw(L"MemberExpressionParser", L"Expected a Identifier");
                    }
                } else {
                    break;
                }
            }
            return Current;
        }
    } // Hoshi
} // Parser