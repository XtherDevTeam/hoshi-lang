//
// Created by p0010 on 22-8-22.
//

#include "AssignmentExpressionParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"
#include "AsExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        AssignmentExpressionParser::AssignmentExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST AssignmentExpressionParser::Parse() {
            AST Id = StaticMemberAccessExpressionParser(L).Parse();
            if (Id.IsNotMatchNode())
                return {};
            AST Op;
            switch (L.LastToken.Kind) {
                case Lexer::TokenKind::AssignSign:
                case Lexer::TokenKind::AdditionAssignment:
                case Lexer::TokenKind::DivisionAssignment:
                case Lexer::TokenKind::MultiplicationAssignment:
                case Lexer::TokenKind::RemainderAssignment:
                case Lexer::TokenKind::SubtractionAssignment: {
                    Op = {AST::TreeType::Operator, L.LastToken};
                    L.Scan();
                    break;
                }
                default: {
                    return {};
                }
            }
            AST Rvalue = AsExpressionParser(L).Parse();
            return {AST::TreeType::AssignmentExpression, {Id, Op, Rvalue}};
        }
    } // Hoshi
} // Parser