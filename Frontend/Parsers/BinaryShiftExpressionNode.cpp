//
// Created by chou on 2022/8/29.
//

#include "BinaryShiftExpressionNode.hpp"

namespace Hoshi {
    BinaryShiftExpressionNode::BinaryShiftExpressionNode(XArray<AdditionExpressionNode *> Operands, XArray<Lexer::Token> Operators)
        : Operands(Operands), Operators(Operators), CSTNode(Operands[0]->Line, Operands[0]->Column) {
    }

    BinaryShiftExpressionNode::BinaryShiftExpressionNode(void)
        : Operands({}), Operators({}), CSTNode(0, 0) {
    }

    BinaryShiftExpressionNode::~BinaryShiftExpressionNode() {
        for (auto *Node : Operands) {
            delete Node;
        }
    }

    XArray<AdditionExpressionNode *> BinaryShiftExpressionNode::GetOperands(void) {
        return Operands;
    }

    AdditionExpressionNode *BinaryShiftExpressionNode::GetOperands(int index) {
        return Operands.at(index);
    }

    XArray<Lexer::Token> BinaryShiftExpressionNode::GetOperators(void) {
        return Operators;
    }

    Lexer::Token BinaryShiftExpressionNode::GetOperators(int index) {
        return Operators.at(index);
    }

    XString BinaryShiftExpressionNode::GetNodeType(void) {
        return L"BinaryShift";
    }

    BinaryShiftExpressionNode::Parser::Parser(void)
        : CSTNode::Parser<BinaryShiftExpressionNode>({BINARYSHIFT_FIRST}) {
    }

    BinaryShiftExpressionNode::Parser BinaryShiftExpressionNode::Parser::INSTANCE;

    BinaryShiftExpressionNode *BinaryShiftExpressionNode::Parser::Parse(Lexer &L) {
        if (! IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except BinaryShift FIRST!");
        XArray<AdditionExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;
        Operands.push_back(AdditionExpressionNode::Parser::INSTANCE.Parse(L));
        bool Flag = false;
        while (true) {
            switch (L.LastToken.Kind) {
            case Lexer::TokenKind::BinaryLeftShift: case Lexer::TokenKind::BinaryRightShift:
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
        BinaryShiftExpressionNode *Result = new BinaryShiftExpressionNode(Operands, Operators);
        return Result;
    }
}