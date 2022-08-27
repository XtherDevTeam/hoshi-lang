//
// Created by Jerry Chou on 22-8-22.
//

#include "SingleExpressionParser.hpp"
#include "PrimaryParser.hpp"

namespace Hoshi {
    namespace Parser {
        SingleExpressionParser::SingleExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST SingleExpressionParser::Parse() {
            AST Operator;
            switch (L.LastToken.Kind) {
                case Lexer::TokenKind::Plus:
                case Lexer::TokenKind::Minus:
                case Lexer::TokenKind::Invert:
                case Lexer::TokenKind::IncrementSign:
                case Lexer::TokenKind::DecrementSign: {
                    Operator = {AST::TreeType::Operator, L.LastToken};
                    L.Scan();
                    break;
                }
                default: {
                    break;
                }
            }
            AST Identifier = PrimaryParser(L).Parse();
            if (Operator.IsNotMatchNode())
                return Identifier;
            return {AST::TreeType::SingleExpression, {Operator, Identifier}};
        }
    } // Hoshi
} // Parser