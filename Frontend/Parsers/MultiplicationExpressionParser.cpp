//
// Created by Jerry Chou on 22-8-22.
//

#include "MultiplicationExpressionParser.hpp"
#include "SingleExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        MultiplicationExpressionParser::MultiplicationExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST MultiplicationExpressionParser::Parse() {
            AST Single = SingleExpressionParser(L).Parse();
            if (Single.IsNotMatchNode())
                return {};
            for (;;) {
                AST Operator;
                switch (L.LastToken.Kind) {
                    case Lexer::TokenKind::Asterisk:
                    case Lexer::TokenKind::Slash:
                    case Lexer::TokenKind::PercentSign: {
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
                AST RightHandSide = SingleExpressionParser(L).Parse();
                if (RightHandSide.IsNotMatchNode()) {
                    Throw (L"MultiplicationExpressionParser", L"Expected a right hand side node");
                    return {};
                }
                Single = {AST::TreeType::MultiplicationExpression, {Single, Operator, RightHandSide}};
            }
            return Single;
        }
    } // Hoshi
} // Parser