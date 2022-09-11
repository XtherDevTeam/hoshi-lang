//
// Created by p0010 on 22-9-11.
//

#include "BinaryExpressionNode.hpp"

namespace Hoshi {
    BinaryExpressionNode::BinaryExpressionNode(XArray<LogicEqualExpressionNode *> Operands,
                                                       XArray<Lexer::Token> Operators)
            : Operands(Operands), Operators(std::move(Operators)), CSTNode(Operands[0]->Line, Operands[0]->Column) {
    }

    BinaryExpressionNode::BinaryExpressionNode()
            : Operands({}), Operators({}), CSTNode(0, 0) {
    }

    BinaryExpressionNode::~BinaryExpressionNode() {
        for (auto *Node: Operands) {
            delete Node;
        }
    }

    XArray<LogicEqualExpressionNode *> BinaryExpressionNode::GetOperands() {
        return Operands;
    }

    LogicEqualExpressionNode *BinaryExpressionNode::GetOperands(int index) {
        return Operands.at(index);
    }

    XArray<Lexer::Token> BinaryExpressionNode::GetOperators() {
        return Operators;
    }

    Lexer::Token BinaryExpressionNode::GetOperators(int index) {
        return Operators.at(index);
    }

    XString BinaryExpressionNode::GetNodeType() {
        return L"Binary";
    }

    BinaryExpressionNode::Parser::Parser()
            : CSTNode::Parser<BinaryExpressionNode>({BINARY_FIRST}) {
    }

    BinaryExpressionNode::Parser BinaryExpressionNode::Parser::INSTANCE;

    BinaryExpressionNode *BinaryExpressionNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except Binary FIRST!");
        XArray<LogicEqualExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;
        Operands.push_back(LogicEqualExpressionNode::Parser::INSTANCE.Parse(L));
        bool Flag;
        while (true) {
            switch (L.LastToken.Kind) {
                case Lexer::TokenKind::BinaryAnd:
                case Lexer::TokenKind::BinaryOr:
                case Lexer::TokenKind::BinaryXOR:
                    Flag = true;
                    break;
                default:
                    Flag = false;
                    break;
            }
            if (!Flag) {
                break;
            }
            Operators.push_back(L.LastToken);
            L.Scan();
            Operands.push_back(LogicEqualExpressionNode::Parser::INSTANCE.Parse(L));
        }
        auto *Result = new BinaryExpressionNode(Operands, Operators);
        return Result;
    }
} // Hoshi