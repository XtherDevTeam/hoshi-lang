//
// Created by chou on 8/31/22.
//

#include "LogicEqualExpressionNode.hpp"

namespace Hoshi {
    LogicEqualExpressionNode::LogicEqualExpressionNode(XArray<LogicComparingExpressionNode *> Operands,
                                                       XArray<Lexer::Token> Operators)
            : Operands(Operands), Operators(std::move(Operators)), CSTNode(Operands[0]->Line, Operands[0]->Column) {
    }

    LogicEqualExpressionNode::LogicEqualExpressionNode()
            : Operands({}), Operators({}), CSTNode(0, 0) {
    }

    LogicEqualExpressionNode::~LogicEqualExpressionNode() {
        for (auto *Node: Operands) {
            delete Node;
        }
    }

    XArray<LogicComparingExpressionNode *> LogicEqualExpressionNode::GetOperands() {
        return Operands;
    }

    LogicComparingExpressionNode *LogicEqualExpressionNode::GetOperands(int index) {
        return Operands.at(index);
    }

    XArray<Lexer::Token> LogicEqualExpressionNode::GetOperators() {
        return Operators;
    }

    Lexer::Token LogicEqualExpressionNode::GetOperators(int index) {
        return Operators.at(index);
    }

    XString LogicEqualExpressionNode::GetNodeType() {
        return L"logicequal";
    }

    LogicEqualExpressionNode::Parser::Parser()
            : CSTNode::Parser<LogicEqualExpressionNode>({LOGICEQUAL_FIRST}) {
    }

    LogicEqualExpressionNode::Parser LogicEqualExpressionNode::Parser::INSTANCE;

    LogicEqualExpressionNode *LogicEqualExpressionNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except LogicEqual FIRST!");
        XArray<LogicComparingExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;
        Operands.push_back(LogicComparingExpressionNode::Parser::INSTANCE.Parse(L));
        bool Flag;
        while (true) {
            switch (L.LastToken.Kind) {
                case Lexer::TokenKind::Equal:
                case Lexer::TokenKind::NotEqual:
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
            Operands.push_back(LogicComparingExpressionNode::Parser::INSTANCE.Parse(L));
        }
        auto *Result = new LogicEqualExpressionNode(Operands, Operators);
        return Result;
    }
} // Hoshi