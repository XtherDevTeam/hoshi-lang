//
// Created by XIaokang00010 on 2022/10/5.
//

#include "ReturnStmtNode.hpp"

#include <utility>

namespace Hoshi {

    ReturnStmtNode::ReturnStmtNode(ExpressionNode *Expr, Lexer::Token Semicolon) :
            Expr(Expr), Semicolon(std::move(Semicolon)), CSTNode(Expr->Line, Expr->Column) {

    }

    ReturnStmtNode::ReturnStmtNode()
            : Expr(nullptr), Semicolon({}), CSTNode(0, 0) {
    }

    ReturnStmtNode::~ReturnStmtNode() {
        if (Expr != nullptr)
            delete Expr;
    }

    ExpressionNode *ReturnStmtNode::GetExpression() {
        return Expr;
    }

    XString ReturnStmtNode::GetNodeType() {
        return L"ReturnStmt";
    }

    ReturnStmtNode::Parser::Parser() : CSTNode::Parser<ReturnStmtNode>({RETURN_STMT_FIRST}) {

    }

    ReturnStmtNode::Parser ReturnStmtNode::Parser::INSTANCE;

    ReturnStmtNode *ReturnStmtNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken)) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"expected return stmt FIRST");
        }
        L.Scan();

        ExpressionNode *Expr = ExpressionNode::Parser::INSTANCE.Parse(L);

        if (Expr == nullptr) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"expected a expression");
        }

        if (L.LastToken.Kind != Lexer::TokenKind::Semicolon) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"expected a semicolon ';'!");
        }

        Lexer::Token Semicolon = L.LastToken;
        L.Scan();

        auto *Result = new ReturnStmtNode(Expr, Semicolon);
        return Result;
    }
} // Hoshi