//
// Created by Jerry Chou on 22-8-22.
//

#include "LogicComparingExpressionParser.hpp"
#include "BinaryMoveExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        LogicComparingExpressionParser::LogicComparingExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST LogicComparingExpressionParser::Parse() {
            AST Single = BinaryMoveExpressionParser(L).Parse();
            if (Single.IsNotMatchNode())
                return {};
            for (;;) {
                AST Operator;
                switch (L.LastToken.Kind) {
                    case Lexer::TokenKind::LessThan:
                    case Lexer::TokenKind::MoreThan:
                    case Lexer::TokenKind::MoreThanOrEqual:
                    case Lexer::TokenKind::LessThanOrEqual:{
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
                AST RightHandSide = BinaryMoveExpressionParser(L).Parse();
                if (RightHandSide.IsNotMatchNode()) {
                    Throw (L"LogicComparingExpressionParser", L"Expected a right hand side node");
                    return {};
                }
                Single = {AST::TreeType::LogicComparingExpression, {Single, Operator, RightHandSide}};
            }
            return Single;
        }
    } // Hoshi
} // Parser