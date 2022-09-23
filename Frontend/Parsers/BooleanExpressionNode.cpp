//
// Created by p0010 on 22-9-23.
//

#include "BooleanExpressionNode.hpp"

namespace Hoshi {
    BooleanExpressionNode::BooleanExpressionNode(XArray<BinaryExpressionNode *> Operands,
                                               XArray<Lexer::Token> Operators)
            : Operands(Operands), Operators(std::move(Operators)), CSTNode(Operands[0]->Line, Operands[0]->Column) {
    }

    BooleanExpressionNode::BooleanExpressionNode()
            : Operands({}), Operators({}), CSTNode(0, 0) {
    }

    BooleanExpressionNode::~BooleanExpressionNode() {
        for (auto *Node: Operands) {
            delete Node;
        }
    }

    XArray<BinaryExpressionNode *> BooleanExpressionNode::GetOperands() {
        return Operands;
    }

    BinaryExpressionNode *BooleanExpressionNode::GetOperands(int index) {
        return Operands.at(index);
    }

    XArray<Lexer::Token> BooleanExpressionNode::GetOperators() {
        return Operators;
    }

    Lexer::Token BooleanExpressionNode::GetOperators(int index) {
        return Operators.at(index);
    }

    XString BooleanExpressionNode::GetNodeType() {
        return L"Boolean";
    }

    BooleanExpressionNode::Parser::Parser()
            : CSTNode::Parser<BooleanExpressionNode>({BOOLEAN_FIRST}) {
    }

    BooleanExpressionNode::Parser BooleanExpressionNode::Parser::INSTANCE;

    BooleanExpressionNode *BooleanExpressionNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except Boolean FIRST!");
        XArray<BinaryExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;
        Operands.push_back(BinaryExpressionNode::Parser::INSTANCE.Parse(L));
        bool Flag;
        while (true) {
            switch (L.LastToken.Kind) {
                case Lexer::TokenKind::LogicAnd:
                case Lexer::TokenKind::LogicOr:
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
            Operands.push_back(BinaryExpressionNode::Parser::INSTANCE.Parse(L));
        }
        auto *Result = new BooleanExpressionNode(Operands, Operators);
        return Result;
    }
} // Hoshi