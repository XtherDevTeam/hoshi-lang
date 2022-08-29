//
// Created by theflysong on 2022/8/29.
//

#include <Parsers/AccessExpressionNode.hpp>

namespace Hoshi {
    AccessExpressionNode::AccessExpressionNode(Lexer::Token Identifier, XArray<AccessOperator> Accesses)
        : Identifier(Identifier), Accesses(Accesses), CSTNode(Identifier.Line, Identifier.Column) {
    }

    AccessExpressionNode::AccessExpressionNode(void)
        : Accesses({}), CSTNode(0, 0) {
    }
    AccessExpressionNode::~AccessExpressionNode() = default;

    XArray<AccessExpressionNode::AccessOperator> AccessExpressionNode::GetAccesses(void) {
        return Accesses;
    }
    
    AccessExpressionNode::AccessOperator AccessExpressionNode::GetAccesses(int index) {
        return Accesses.at(index);
    }

    Lexer::Token AccessExpressionNode::GetIdentifier(void) {
        return Identifier;
    }

    XString AccessExpressionNode::GetNodeType(void) {
        return L"access";
    }

    AccessExpressionNode::Parser::Parser(void) 
        : CSTNode::Parser<AccessExpressionNode>({ACCESS_FIRST}) {
    }

    AccessExpressionNode::Parser AccessExpressionNode::Parser::INSTANCE;

    AccessExpressionNode *AccessExpressionNode::Parser::Parse(Lexer &L) {
        if (! IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except access FIRST!");
        XArray<AccessOperator> Operators;

        Lexer::Token Identifier = L.LastToken;
        L.Scan();
        while (true) {
            if (L.LastToken.Kind == Lexer::TokenKind::Dot) {
                Lexer::Token Dot = L.LastToken;
                L.Scan();
                if (L.LastToken.Kind != Lexer::TokenKind::Identifier)
                    throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except an Identifier");
                
                Lexer::Token Identifier = L.LastToken;
                L.Scan();
                Operators.push_back(AccessOperator{.Dot = Dot, .Identifier = Identifier});
            }
            else {
                break;
            }
        }
        AccessExpressionNode *Result = new AccessExpressionNode(Identifier, Operators);
        return Result;
    }
}