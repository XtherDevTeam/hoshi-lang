//
// Created by theflysong on 2022/8/28.
//

#include <Parsers/LiteralsNode.hpp>
#include <Exceptions/ParserException.hpp>

namespace Hoshi {
    LiteralsNode::LiteralsNode(void) = default;
    
    LiteralsNode::LiteralsNode(Lexer::Token Literals) 
        : Literals(Literals) {
    }

    Lexer::Token LiteralsNode::GetLiterals(void) {
        return Literals;
    }

    XString LiteralsNode::GetNodeType(void) {
        return L"literals";
    }

    LiteralsNode::Parser::Parser(void) = default;

    LiteralsNode::Parser LiteralsNode::Parser::INSTANCE;

    LiteralsNode LiteralsNode::Parser::Parse(Lexer L) {
        switch (L.LastToken.Kind) {
            //Literals
        case Lexer::TokenKind::Integer:
        case Lexer::TokenKind::Boolean:
        case Lexer::TokenKind::Decimal:
        case Lexer::TokenKind::String:
        case Lexer::TokenKind::Character: {
            LiteralsNode Res(L.LastToken);
            L.Scan();
            return Res;
        }
        default: {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except a literal!");
        }
        }
    }
}