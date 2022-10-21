//
// Created by theflysong on 2022/10/21.
//

#include "StmtNode.hpp"

namespace Hoshi {
    StmtNode::StmtNode(ReturnStmtNode *ReturnStmt) 
        : ReturnStmt(nullptr), ExpressionStmt(nullptr), AssignStmt(nullptr),
          BreakStmt(nullptr), ContinueStmt(nullptr), CSTNode(0, 0)
    {
    }
    StmtNode::StmtNode(ExpressionStmtNode *ExpressionStmt) 
        : ReturnStmt(nullptr), ExpressionStmt(nullptr), AssignStmt(nullptr),
          BreakStmt(nullptr), ContinueStmt(nullptr), CSTNode(0, 0)
    {
    }
    StmtNode::StmtNode(AssignStmtNode *AssignStmt) 
        : ReturnStmt(nullptr), ExpressionStmt(nullptr), AssignStmt(nullptr),
          BreakStmt(nullptr), ContinueStmt(nullptr), CSTNode(0, 0)
    {
    }
    StmtNode::StmtNode(BreakStmtNode *BreakStmt) 
        : ReturnStmt(nullptr), ExpressionStmt(nullptr), AssignStmt(nullptr),
          BreakStmt(nullptr), ContinueStmt(nullptr), CSTNode(0, 0)
    {
    }
    StmtNode::StmtNode(ContinueStmtNode *ContinueStmt) 
        : ReturnStmt(nullptr), ExpressionStmt(nullptr), AssignStmt(nullptr),
          BreakStmt(nullptr), ContinueStmt(nullptr), CSTNode(0, 0)
    {
    }
    StmtNode::StmtNode() 
        : ReturnStmt(nullptr), ExpressionStmt(nullptr), AssignStmt(nullptr),
          BreakStmt(nullptr), ContinueStmt(nullptr), CSTNode(0, 0)
    {
    }
    StmtNode::~StmtNode() {
        if (ReturnStmt != nullptr) delete ReturnStmt;
        if (ExpressionStmt != nullptr) delete ExpressionStmt;
        if (AssignStmt != nullptr) delete AssignStmt;
        if (BreakStmt != nullptr) delete BreakStmt;
        if (ContinueStmt != nullptr) delete ContinueStmt;
    }
    XString StmtNode::GetNodeType() {
        return L"stmt";
    }
    StmtNode::Parser::Parser()
            : CSTNode::Parser<StmtNode>({STMT_FIRST}) {
    }

    StmtNode::Parser StmtNode::Parser::INSTANCE;

    StmtNode *StmtNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken)) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"expected stmt FIRST");
        }

        if (ReturnStmtNode::Parser::INSTANCE.IsFirstToken(L.LastToken)) {
            ReturnStmtNode *ReturnStmt = ReturnStmtNode::Parser::INSTANCE.Parse(L);
            return new StmtNode(ReturnStmt);
        }

        if (ExpressionStmtNode::Parser::INSTANCE.IsFirstToken(L.LastToken)) {
            ExpressionStmtNode *ExpressionStmt = ExpressionStmtNode::Parser::INSTANCE.Parse(L);
            return new StmtNode(ExpressionStmt);
        }

        if (AssignStmtNode::Parser::INSTANCE.IsFirstToken(L.LastToken)) {
            AssignStmtNode *AssignStmt = AssignStmtNode::Parser::INSTANCE.Parse(L);
            return new StmtNode(AssignStmt);
        }

        if (BreakStmtNode::Parser::INSTANCE.IsFirstToken(L.LastToken)) {
            BreakStmtNode *BreakStmt = BreakStmtNode::Parser::INSTANCE.Parse(L);
            return new StmtNode(BreakStmt);
        }
        if (ContinueStmtNode::Parser::INSTANCE.IsFirstToken(L.LastToken)) {
            ContinueStmtNode *ContinueStmt = ContinueStmtNode::Parser::INSTANCE.Parse(L);
            return new StmtNode(ContinueStmt);
        }

        throw ParserException(L.LastToken.Line, L.LastToken.Column, L"unsupported stmt");
    }
}