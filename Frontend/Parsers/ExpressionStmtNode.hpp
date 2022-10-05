//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_EXPRESSIONSTMTNODE_HPP
#define XSCRIPT2_EXPRESSIONSTMTNODE_HPP

#include <Parsers/ExpressionNode.hpp>

namespace Hoshi {
    class ExpressionStmtNode : public CSTNode {
        ExpressionNode *Expression;
        Lexer::Token Semicolon;

        ExpressionStmtNode(ExpressionNode *Expression, Lexer::Token Semicolon);

    public:
        ExpressionStmtNode();

        ~ExpressionStmtNode() override;

        ExpressionNode *GetExpression();

        Lexer::Token GetSemicolon();

        XString GetNodeType() override;

#define EXPRESSION_STMT_FIRST EXPRESSION_FIRST

        class Parser : public CSTNode::Parser<ExpressionStmtNode> {
            Parser();

        public:
            static Parser INSTANCE;

            ExpressionStmtNode *Parse(Lexer &L) override;
        };
    };
}

#endif