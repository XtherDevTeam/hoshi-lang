//
// Created by theflysong on 2022/8/29.
//

#include <Parsers/MuliplicationExpressionNode.hpp>
#include <utility>

namespace Hoshi {
    MuliplicationExpressionNode::MuliplicationExpressionNode(XArray<UniqueExpressionNode *> Operands,
                                                             XArray<Lexer::Token> Operators)
            : Operands(Operands), Operators(std::move(Operators)), CSTNode(Operands[0]->Line, Operands[0]->Column) {
    }

    MuliplicationExpressionNode::MuliplicationExpressionNode()
            : Operands({}), Operators({}), CSTNode(0, 0) {
    }

    MuliplicationExpressionNode::~MuliplicationExpressionNode() {
        for (auto *Node: Operands) {
            delete Node;
        }
    }

    XArray<UniqueExpressionNode *> MuliplicationExpressionNode::GetOperands() {
        return Operands;
    }

    UniqueExpressionNode *MuliplicationExpressionNode::GetOperands(int index) {
        return Operands.at(index);
    }

    XArray<Lexer::Token> MuliplicationExpressionNode::GetOperators() {
        return Operators;
    }

    Lexer::Token MuliplicationExpressionNode::GetOperators(int index) {
        return Operators.at(index);
    }

    XString MuliplicationExpressionNode::GetNodeType() {
        return L"muliplication";
    }

    MuliplicationExpressionNode::Parser::Parser()
            : CSTNode::Parser<MuliplicationExpressionNode>({MULIPLICATION_FIRST}) {
    }

    MuliplicationExpressionNode::Parser MuliplicationExpressionNode::Parser::INSTANCE;

    MuliplicationExpressionNode *MuliplicationExpressionNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except muliplication FIRST!");
        XArray<UniqueExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;
        Operands.push_back(UniqueExpressionNode::Parser::INSTANCE.Parse(L));
        bool Flag = false;
        while (true) {
            switch (L.LastToken.Kind) {
                case Lexer::TokenKind::Asterisk:
                case Lexer::TokenKind::Slash:
                case Lexer::TokenKind::PercentSign:
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
            Operands.push_back(UniqueExpressionNode::Parser::INSTANCE.Parse(L));
        }
        auto *Result = new MuliplicationExpressionNode(Operands, Operators);
        return Result;
    }
}