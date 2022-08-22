//
// Created by Jerry Chou on 22-8-22.
//

#include "LogicEqualExpressionParser.hpp"
#include "LogicComparingExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        LogicEqualExpressionParser::LogicEqualExpressionParser(Hoshi::Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST LogicEqualExpressionParser::Parse() {
            AST Single = LogicComparingExpressionParser(L).Parse();
            if (Single.IsNotMatchNode())
                return {};
            for (;;) {
                AST Operator;
                switch (L.LastToken.Kind) {
                    case Lexer::TokenKind::Equal:
                    case Lexer::TokenKind::NotEqual: {
                        Operator = {AST::TreeType::Operator, L.LastToken};
                        L.Scan();
                        break;
                    }
                    default: {
                        break;
                    }
                }
                if (Operator.IsNotMatchNode())
                    break;
                AST RightHandSide = LogicComparingExpressionParser(L).Parse();
                if (RightHandSide.IsNotMatchNode()) {
                    Throw(L"LogicComparingExpressionParser", L"Expected a right hand side node");
                    return {};
                }
                Single = {AST::TreeType::LogicEqualExpression, {Single, Operator, RightHandSide}};
            }
            return Single;
        }
    } // Hoshi
} // Parser