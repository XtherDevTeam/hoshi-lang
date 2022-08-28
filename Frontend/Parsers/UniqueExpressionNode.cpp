//
// Created by theflysong on 2022/8/28.
//

#include <Parsers/UniqueExpressionNode.hpp>

namespace Hoshi {
    UniqueExpressionNode::UniqueExpressionNode(PrimaryExpressionNode *Primary) 
        : Operator({}), Primary(Primary) {
    }

    UniqueExpressionNode::UniqueExpressionNode(Lexer::Token Operator, PrimaryExpressionNode *Primary) 
        : Operator(Operator), Primary(Primary){
    }

    UniqueExpressionNode::UniqueExpressionNode(void)
        : Operator({}), Primary(Primary) {
    }

    UniqueExpressionNode::~UniqueExpressionNode() {
        if (Primary != NULL)
            delete Primary;
    }

    Lexer::Token UniqueExpressionNode::GetOperator(void) {
        return Operator;
    }

    PrimaryExpressionNode *UniqueExpressionNode::GetPrimary(void) {
        return Primary;
    }

    XString UniqueExpressionNode::GetNodeType(void) {
        return L"unique";
    }

    UniqueExpressionNode::Parser::Parser(void)
        : CSTNode::Parser<UniqueExpressionNode>({UNIQUE_FIRST}) {
    }

    UniqueExpressionNode::Parser UniqueExpressionNode::Parser::INSTANCE;
            
    UniqueExpressionNode *UniqueExpressionNode::Parser::Parse(Lexer &L) {
        if (! IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except unique expression FIRST!");

        if (PrimaryExpressionNode::Parser::INSTANCE.IsFirstToken(L.LastToken)) {
            PrimaryExpressionNode *Primary = PrimaryExpressionNode::Parser::INSTANCE.Parse(L);
            UniqueExpressionNode *Unique = new UniqueExpressionNode(Primary);
            return Unique;
        }

        Lexer::Token Operator = L.LastToken;
        L.Scan();
        PrimaryExpressionNode *Primary = PrimaryExpressionNode::Parser::INSTANCE.Parse(L);
        UniqueExpressionNode *Unique = new UniqueExpressionNode(Operator, Primary);
        return Unique;
    }
}