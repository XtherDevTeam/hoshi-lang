//
// Created by Jerry Chou on 22-8-22.
//

#include "BinaryExpressionParser.hpp"
#include "LogicComparingExpressionParser.hpp"
#include "LogicEqualExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        BinaryExpressionParser::BinaryExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST BinaryExpressionParser::Parse() {
            AST Single = LogicEqualExpressionParser(L).Parse();
            if (Single.IsNotMatchNode())
                return {};
            for (;;) {
                AST Operator;
                switch (L.LastToken.Kind) {
                    case Lexer::TokenKind::BinaryAnd:
                    case Lexer::TokenKind::BinaryXOR:
                    case Lexer::TokenKind::BinaryOr:{
                        Operator = {AST::TreeType::Operator, L.LastToken};
                        L.Scan();
                        break;
                    }
                    case Lexer::TokenKind::Keywords:{
                        if (L.LastToken.Value != L"instanceOf" and L.LastToken.Value != L"implemented") {
                            return Single;
                        }
                        Operator = {AST::TreeType::Operator, L.LastToken};
                        L.Scan();
                        break;
                    }
                    default: {
                        return Single;
                    }
                }
                if (Operator.IsNotMatchNode())
                    break;
                AST RightHandSide = LogicComparingExpressionParser(L).Parse();
                if (RightHandSide.IsNotMatchNode()) {
                    Throw (L"BinaryExpression", L"Expected a right hand side node");
                    return {};
                }
                Single = {AST::TreeType::BinaryExpression, {Single, Operator, RightHandSide}};
            }
            return Single;
        }
    } // Hoshi
} // Parser