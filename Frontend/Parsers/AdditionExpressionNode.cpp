//
// Created by theflysong on 2022/8/29.
//

#include <Parsers/AdditionExpressionNode.hpp>

namespace Hoshi {
    AdditionExpressionNode::AdditionExpressionNode(XArray<MuliplicationExpressionNode *> Operands, XArray<Lexer::Token> Operators)
        : Operands(Operands), Operators(Operators), CSTNode(Operands[0]->Line, Operands[0]->Column) {
    }
   
    AdditionExpressionNode::AdditionExpressionNode(void)
        : Operands({}), Operators({}), CSTNode(0, 0) {
    }

    AdditionExpressionNode::~AdditionExpressionNode() {
        for (auto *Node : Operands) {
            delete Node;
        }
    }

    XArray<MuliplicationExpressionNode *> AdditionExpressionNode::GetOperands(void) {
        return Operands;
    }

    MuliplicationExpressionNode *AdditionExpressionNode::GetOperands(int index) {
        return Operands.at(index);
    }

    XArray<Lexer::Token> AdditionExpressionNode::GetOperators(void) {
        return Operators;
    }

    Lexer::Token AdditionExpressionNode::GetOperators(int index) {
        return Operators.at(index);
    }

    XString AdditionExpressionNode::GetNodeType(void) {
        return L"addition";
    }

    AdditionExpressionNode::Parser::Parser(void) 
        : CSTNode::Parser<AdditionExpressionNode>({MULIPLICATION_FIRST}) {
    }

    AdditionExpressionNode::Parser AdditionExpressionNode::Parser::INSTANCE;

    AdditionExpressionNode *AdditionExpressionNode::Parser::Parse(Lexer &L) {
        if (! IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except addition FIRST!");
        XArray<MuliplicationExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;
        Operands.push_back(MuliplicationExpressionNode::Parser::INSTANCE.Parse(L));
        bool Flag = false;
        while (true) {
            switch (L.LastToken.Kind) {
            case Lexer::TokenKind::Plus: case Lexer::TokenKind::Minus:
                Flag = true;
                break;
            default: 
                Flag = false;
                break;
            }
            if (! Flag) {
                break;
            }
            Operators.push_back(L.LastToken);
            L.Scan();
            Operands.push_back(MuliplicationExpressionNode::Parser::INSTANCE.Parse(L));
        }
        AdditionExpressionNode *Result = new AdditionExpressionNode(Operands, Operators);
        return Result;
    }
}