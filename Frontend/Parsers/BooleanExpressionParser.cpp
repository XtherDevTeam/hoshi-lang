//
// Created by Jerry Chou on 22-8-22.
//

#include "BooleanExpressionParser.hpp"
#include "BinaryExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        BooleanExpressionParser::BooleanExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST BooleanExpressionParser::Parse() {
            AST Single = BinaryExpressionParser(L).Parse();
            if (Single.IsNotMatchNode())
                return {};
            for (;;) {
                AST Operator;
                switch (L.LastToken.Kind) {
                    case Lexer::TokenKind::LogicAnd:
                    case Lexer::TokenKind::LogicOr: {
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
                AST RightHandSide = BinaryExpressionParser(L).Parse();
                if (RightHandSide.IsNotMatchNode()) {
                    Throw(L"BooleanExpressionParser", L"Expected a right hand side node");
                    return {};
                }
                Single = {AST::TreeType::BooleanExpression, {Single, Operator, RightHandSide}};
            }
            return Single;
        }
    } // Hoshi
} // Parser