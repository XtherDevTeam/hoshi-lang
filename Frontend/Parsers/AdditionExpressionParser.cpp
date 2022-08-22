//
// Created by Jerry Chou on 22-8-22.
//

#include "AdditionExpressionParser.hpp"
#include "MultiplicationExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        AdditionExpressionParser::AdditionExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST AdditionExpressionParser::Parse() {
            AST Single = MultiplicationExpressionParser(L).Parse();
            if (Single.IsNotMatchNode())
                return {};
            for (;;) {
                AST Operator;
                switch (L.LastToken.Kind) {
                    case Lexer::TokenKind::Plus:
                    case Lexer::TokenKind::Minus: {
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
                AST RightHandSide = MultiplicationExpressionParser(L).Parse();
                if (RightHandSide.IsNotMatchNode()) {
                    Throw (L"MultiplicationExpressionParser", L"Expected a right hand side node");
                    return {};
                }
                Single = {AST::TreeType::AdditionExpression, {Single, Operator, RightHandSide}};
            }
            return Single;
        }
    } // Hoshi
} // Parser