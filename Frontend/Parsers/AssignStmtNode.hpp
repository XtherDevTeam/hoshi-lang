//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_ASSIGNSTMTNODE_HPP
#define XSCRIPT2_ASSIGNSTMTNODE_HPP

#include <Parsers/ExpressionNode.hpp>

namespace Hoshi {
    class AssignStmtNode : public CSTNode {
        AccessExpressionNode *Access;
        Lexer::Token AssignOperator;
        ExpressionNode *Expression;
        Lexer::Token Semicolon;

        AssignStmtNode(AccessExpressionNode *Access, Lexer::Token AssignOperator, ExpressionNode *Expression,
                       Lexer::Token Semicolon);

    public:
        AssignStmtNode();

        virtual ~AssignStmtNode();

        AccessExpressionNode *GetAccess();

        Lexer::Token GetAssignOperator();

        ExpressionNode *GetExpression();

        XString GetNodeType() override;

#define ASSIGN_FIRST ACCESS_FIRST

        class Parser : public CSTNode::Parser<AssignStmtNode> {
            Parser();

        public:
            static Parser INSTANCE;

            bool IsAssignStmt(Lexer L);
            virtual AssignStmtNode *Parse(Lexer &L) override;
        };
    };
}

#endif
