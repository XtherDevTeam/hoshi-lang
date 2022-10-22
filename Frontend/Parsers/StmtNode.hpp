//
// Created by theflysong on 2022/10/21.
//

#ifndef XSCRIPT2_STMTNODE_HPP
#define XSCRIPT2_STMTNODE_HPP

#include "ReturnStmtNode.hpp"
#include "ExpressionStmtNode.hpp"
#include "AssignStmtNode.hpp"
#include "BreakStmtNode.hpp"
#include "ContinueStmtNode.hpp"

namespace Hoshi {
    class StmtNode : public CSTNode {
        ReturnStmtNode *ReturnStmt;
        ExpressionStmtNode *ExpressionStmt;
        AssignStmtNode *AssignStmt;
        BreakStmtNode *BreakStmt;
        ContinueStmtNode *ContinueStmt;

        StmtNode(ReturnStmtNode *ReturnStmt);
        StmtNode(ExpressionStmtNode *ExpressionStmt);
        StmtNode(AssignStmtNode *AssignStmt);
        StmtNode(BreakStmtNode *BreakStmt);
        StmtNode(ContinueStmtNode *ContinueStmt);
    public:
        StmtNode();

        ~StmtNode() override;

        ReturnStmtNode *GetReturnStmt(void);
        ExpressionStmtNode *GetExpressionStmt(void);
        AssignStmtNode *GetAssignStmt(void);
        BreakStmtNode *GetBreakStmt(void);
        ContinueStmtNode *GetContinueStmt(void);

        XString GetNodeType() override;

#define STMT_FIRST RETURN_STMT_FIRST, \
                   EXPRESSION_STMT_FIRST, \
                   ASSIGN_FIRST, \
                   BREAK_STMT_FIRST, \
                   CONTINUE_STMT_FIRST

        class Parser : public CSTNode::Parser<StmtNode> {
            Parser();

        public:
            static Parser INSTANCE;

            StmtNode *Parse(Lexer &L) override;
        };
    };
}

#endif