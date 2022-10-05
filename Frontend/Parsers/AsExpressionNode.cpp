//
// Created by p0010 on 22-9-23.
//

#include "AsExpressionNode.hpp"

namespace Hoshi {
    AsExpressionNode::AsExpressionNode(XArray<BooleanExpressionNode *> Operands,
                                       XArray<Lexer::Token> Operators)
            : Operands(Operands), Operators(std::move(Operators)), CSTNode(Operands[0]->Line, Operands[0]->Column) {
    }

    AsExpressionNode::AsExpressionNode()
            : Operands({}), Operators({}), CSTNode(0, 0) {
    }

    AsExpressionNode::~AsExpressionNode() {
        for (auto *Node: Operands) {
            delete Node;
        }
    }

    XArray<BooleanExpressionNode *> AsExpressionNode::GetOperands() {
        return Operands;
    }

    BooleanExpressionNode *AsExpressionNode::GetOperands(int index) {
        return Operands.at(index);
    }

    XArray<Lexer::Token> AsExpressionNode::GetOperators() {
        return Operators;
    }

    Lexer::Token AsExpressionNode::GetOperators(int index) {
        return Operators.at(index);
    }

    XString AsExpressionNode::GetNodeType() {
        return L"As";
    }

    AsExpressionNode::Parser::Parser()
            : CSTNode::Parser<AsExpressionNode>({AS_FIRST}) {
    }

    AsExpressionNode::Parser AsExpressionNode::Parser::INSTANCE;

    AsExpressionNode *AsExpressionNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except As FIRST!");
        XArray<BooleanExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;
        Operands.push_back(BooleanExpressionNode::Parser::INSTANCE.Parse(L));
        bool Flag;
        while (true) {
            Flag = (L.LastToken.Kind == Lexer::TokenKind::Keywords && L.LastToken.Value == L"as");
            if (!Flag) {
                break;
            }
            Operators.push_back(L.LastToken);
            L.Scan();
            Operands.push_back(BooleanExpressionNode::Parser::INSTANCE.Parse(L));
        }
        auto *Result = new AsExpressionNode(Operands, Operators);
        return Result;
    }
} // Hoshi