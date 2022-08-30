//
// Created by chou on 8/30/22.
//

#include "LogicComparingExpressionNode.hpp"


#include <utility>

namespace Hoshi {
    LogicComparingExpressionNode::LogicComparingExpressionNode(XArray<BinaryShiftExpressionNode *> Operands,
                                                               XArray<Lexer::Token> Operators)
            : Operands(Operands), Operators(std::move(Operators)), CSTNode(Operands[0]->Line, Operands[0]->Column) {
    }

    LogicComparingExpressionNode::LogicComparingExpressionNode()
            : Operands({}), Operators({}), CSTNode(0, 0) {
    }

    LogicComparingExpressionNode::~LogicComparingExpressionNode() {
        for (auto *Node: Operands) {
            delete Node;
        }
    }

    XArray<BinaryShiftExpressionNode *> LogicComparingExpressionNode::GetOperands() {
        return Operands;
    }

    BinaryShiftExpressionNode *LogicComparingExpressionNode::GetOperands(int index) {
        return Operands.at(index);
    }

    XArray<Lexer::Token> LogicComparingExpressionNode::GetOperators() {
        return Operators;
    }

    Lexer::Token LogicComparingExpressionNode::GetOperators(int index) {
        return Operators.at(index);
    }

    XString LogicComparingExpressionNode::GetNodeType() {
        return L"logiccomparing";
    }

    LogicComparingExpressionNode::Parser::Parser()
            : CSTNode::Parser<LogicComparingExpressionNode>({LOGICCOMPARING_FIRST}) {
    }

    LogicComparingExpressionNode::Parser LogicComparingExpressionNode::Parser::INSTANCE;

    LogicComparingExpressionNode *LogicComparingExpressionNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except LogicComparing FIRST!");
        XArray<BinaryShiftExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;
        Operands.push_back(BinaryShiftExpressionNode::Parser::INSTANCE.Parse(L));
        bool Flag;
        while (true) {
            switch (L.LastToken.Kind) {
                case Lexer::TokenKind::LessThan:
                case Lexer::TokenKind::LessThanOrEqual:
                case Lexer::TokenKind::GreaterThan:
                case Lexer::TokenKind::GreaterThanOrEqual:
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
            Operands.push_back(BinaryShiftExpressionNode::Parser::INSTANCE.Parse(L));
        }
        auto *Result = new LogicComparingExpressionNode(Operands, Operators);
        return Result;
    }
}