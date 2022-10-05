//
// Created by theflysong on 2022/8/28.
//

#include <Parsers/UniqueExpressionNode.hpp>

namespace Hoshi {
    UniqueExpressionNode::UniqueExpressionNode(PrimaryExpressionNode *Primary)
            : Operator({}), Primary(Primary), CSTNode(Primary->Line, Primary->Column) {
    }

    UniqueExpressionNode::UniqueExpressionNode(Lexer::Token Operator, PrimaryExpressionNode *Primary)
            : Operator(Operator), Primary(Primary), CSTNode(Operator.Line, Operator.Column) {
    }

    UniqueExpressionNode::UniqueExpressionNode()
            : Operator({}), Primary(nullptr), CSTNode(0, 0) {
    }

    UniqueExpressionNode::~UniqueExpressionNode() {
        if (Primary != nullptr)
            delete Primary;
    }

    Lexer::Token UniqueExpressionNode::GetOperator() {
        return Operator;
    }

    PrimaryExpressionNode *UniqueExpressionNode::GetPrimary() {
        return Primary;
    }

    XString UniqueExpressionNode::GetNodeType() {
        return L"unique";
    }

    UniqueExpressionNode::Parser::Parser()
            : CSTNode::Parser<UniqueExpressionNode>({UNIQUE_FIRST}) {
    }

    UniqueExpressionNode::Parser UniqueExpressionNode::Parser::INSTANCE;

    UniqueExpressionNode *UniqueExpressionNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except unique expression FIRST!");

        if (PrimaryExpressionNode::Parser::INSTANCE.IsFirstToken(L.LastToken)) {
            PrimaryExpressionNode *Primary = PrimaryExpressionNode::Parser::INSTANCE.Parse(L);
            auto *Unique = new UniqueExpressionNode(Primary);
            return Unique;
        }

        Lexer::Token Operator = L.LastToken;
        L.Scan();
        PrimaryExpressionNode *Primary = PrimaryExpressionNode::Parser::INSTANCE.Parse(L);
        auto *Unique = new UniqueExpressionNode(Operator, Primary);
        return Unique;
    }
}