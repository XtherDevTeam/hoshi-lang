//
// Created by Jerry Chou on 22-8-22.
//

#include "BinaryMoveExpressionParser.hpp"
#include "AdditionExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        BinaryMoveExpressionParser::BinaryMoveExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST BinaryMoveExpressionParser::Parse() {
            AST Single = AdditionExpressionParser(L).Parse();
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
                AST RightHandSide = AdditionExpressionParser(L).Parse();
                if (RightHandSide.IsNotMatchNode()) {
                    Throw (L"BinaryMoveExpression", L"Expected a right hand side node");
                    return {};
                }
                Single = {AST::TreeType::BinaryMoveExpression, {Single, Operator, RightHandSide}};
            }
            return Single;
        }
    } // Hoshi
} // Parser