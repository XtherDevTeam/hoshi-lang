//
// Created by theflysong on 2022/8/29.
//

#include "BinaryMoveExpressionNode.hpp"

namespace Hoshi {
    BinaryMoveExpressionNode::BinaryMoveExpressionNode(XArray<AdditionExpressionNode *> Operands, XArray<Lexer::Token> Operators)
        : Operands(Operands), Operators(Operators), CSTNode(Operands[0]->Line, Operands[0]->Column) {
    }

    BinaryMoveExpressionNode::BinaryMoveExpressionNode(void)
        : Operands({}), Operators({}), CSTNode(0, 0) {
    }

    BinaryMoveExpressionNode::~BinaryMoveExpressionNode() {
        for (auto *Node : Operands) {
            delete Node;
        }
    }

    XArray<AdditionExpressionNode *> BinaryMoveExpressionNode::GetOperands(void) {
        return Operands;
    }

    AdditionExpressionNode *BinaryMoveExpressionNode::GetOperands(int index) {
        return Operands.at(index);
    }

    XArray<Lexer::Token> BinaryMoveExpressionNode::GetOperators(void) {
        return Operators;
    }

    Lexer::Token BinaryMoveExpressionNode::GetOperators(int index) {
        return Operators.at(index);
    }

    XString BinaryMoveExpressionNode::GetNodeType(void) {
        return L"BinaryMove";
    }

    BinaryMoveExpressionNode::Parser::Parser(void)
        : CSTNode::Parser<BinaryMoveExpressionNode>({BINARYMOVE_FIRST}) {
    }

    BinaryMoveExpressionNode::Parser BinaryMoveExpressionNode::Parser::INSTANCE;

    BinaryMoveExpressionNode *BinaryMoveExpressionNode::Parser::Parse(Lexer &L) {
        if (! IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except BinaryMove FIRST!");
        XArray<AdditionExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;
        Operands.push_back(AdditionExpressionNode::Parser::INSTANCE.Parse(L));
        bool Flag = false;
        while (true) {
            switch (L.LastToken.Kind) {
            case Lexer::TokenKind::BinaryLeftMove: case Lexer::TokenKind::BinaryRightMove:
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
            Operands.push_back(AdditionExpressionNode::Parser::INSTANCE.Parse(L));
        }
        BinaryMoveExpressionNode *Result = new BinaryMoveExpressionNode(Operands, Operators);
        return Result;
    }
}