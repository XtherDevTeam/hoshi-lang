//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_UNIQUEEXPRESSIONNODE_HPP
#define XSCRIPT2_UNIQUEEXPRESSIONNODE_HPP

#include <Parsers/PrimaryExpressionNode.hpp>

namespace Hoshi {
    class UniqueExpressionNode : public CSTNode {
        Lexer::Token Operator;
        PrimaryExpressionNode *Primary;

        UniqueExpressionNode(PrimaryExpressionNode *Primary);

        UniqueExpressionNode(Lexer::Token Operator, PrimaryExpressionNode *Primary);

    public:
        UniqueExpressionNode();

        ~UniqueExpressionNode() override;

        Lexer::Token GetOperator();

        PrimaryExpressionNode *GetPrimary();

        XString GetNodeType() override;

#define UNIQUE_FIRST PRIMARY_FIRST, \
            Lexer::TokenKind::Plus, \
            Lexer::TokenKind::Minus, \
            Lexer::TokenKind::Invert, \
            Lexer::TokenKind::IncrementSign, \
            Lexer::TokenKind::DecrementSign

        class Parser : public CSTNode::Parser<UniqueExpressionNode> {
            Parser();

        public:
            static Parser INSTANCE;

            UniqueExpressionNode *Parse(Lexer &L) override;
        };
    };
}

#endif