//
// Created by theflysong on 2022/8/29.
//

#include <Parsers/ExpressionStmtNode.hpp>
#include <Parsers/AssignStmtNode.hpp>

namespace Hoshi {
    ExpressionStmtNode::ExpressionStmtNode(ExpressionNode *Expression, Lexer::Token Semicolon)
        : Expression(Expression), Semicolon(Semicolon), CSTNode(Expression->Line, Expression->Column) {
    }

    ExpressionStmtNode::ExpressionStmtNode()
            : Expression(NULL), Semicolon({}), CSTNode(0, 0) {
    }
    ExpressionStmtNode::~ExpressionStmtNode() {
        if (Expression != NULL)
            delete Expression;
    }

    ExpressionNode *ExpressionStmtNode::GetExpression() {
        return Expression;
    }

    Lexer::Token ExpressionStmtNode::GetSemicolon() {
        return Semicolon;
    }

    XString ExpressionStmtNode::GetNodeType() {
        return L"expression stmt";
    }

    ExpressionStmtNode::Parser::Parser()
            : CSTNode::Parser<ExpressionStmtNode>({EXPRESSION_STMT_FIRST}) {
    }

    ExpressionStmtNode::Parser ExpressionStmtNode::Parser::INSTANCE;

    ExpressionStmtNode *ExpressionStmtNode::Parser::Parse(Lexer &L) {
        if (! IsFirstToken(L.LastToken)) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"except expression stmt FIRST");
        }

        if (AssignStmtNode::Parser::INSTANCE.IsAssignStmt(L)) {
            return NULL; //是Assign Stmt
        }

        ExpressionNode *Expression = ExpressionNode::Parser::INSTANCE.Parse(L);
        
        if (L.LastToken.Kind != Lexer::TokenKind::Semicolon) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"except a semicolon ';'!");
        }

        Lexer::Token Semicolon = L.LastToken;
        L.Scan();

        ExpressionStmtNode *Result = new ExpressionStmtNode(Expression, Semicolon);
        return Result;
    }
}