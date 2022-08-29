//
// Created by theflysong on 2022/8/29.
//

#include <Parsers/AssignStmtNode.hpp>

namespace Hoshi {
    AssignStmtNode::AssignStmtNode(AccessExpressionNode *Access, Lexer::Token AssignOperator, ExpressionNode *Expression, Lexer::Token Semicolon)
        : Access(Access), AssignOperator(AssignOperator), Expression(Expression), 
          Semicolon(Semicolon), CSTNode(Access->Line, Access->Column) {
    }

    AssignStmtNode::AssignStmtNode(void)
        : Access(NULL), AssignOperator({}), Expression(NULL), CSTNode(0, 0) {
    }

    AssignStmtNode::~AssignStmtNode() {
        if (Access != NULL)
            delete Access;
        if (Expression != NULL)
            delete Expression;
    }

    AccessExpressionNode *AssignStmtNode::GetAccess(void) {
        return Access;
    }

    Lexer::Token AssignStmtNode::GetAssignOperator(void) {
        return AssignOperator;
    }

    ExpressionNode *AssignStmtNode::GetExpression(void) {
        return Expression;
    }

    XString AssignStmtNode::GetNodeType(void) {
        return L"assign stmt";
    }

    AssignStmtNode::Parser::Parser(void) 
        : CSTNode::Parser<AssignStmtNode>({ASSIGN_FIRST}) {
    }

    AssignStmtNode::Parser AssignStmtNode::Parser::INSTANCE;

    bool AssignStmtNode::Parser::IsAssignStmt(Lexer L) {
        AccessExpressionNode *Access = AccessExpressionNode::Parser::INSTANCE.Parse(L);

        bool Flag;
        switch (L.LastToken.Kind) {
        case Lexer::TokenKind::AssignSign: case Lexer::TokenKind::RemainderAssignment:
        case Lexer::TokenKind::AdditionAssignment: case Lexer::TokenKind::SubtractionAssignment:
        case Lexer::TokenKind::MultiplicationAssignment: case Lexer::TokenKind::DivisionAssignment:
            Flag = true;
            break;
        default:
            Flag = false;
            break;
        }

        if (Access != NULL)
            delete Access;

        return Flag;
    }

    AssignStmtNode *AssignStmtNode::Parser::Parse(Lexer &L) {
        if (! IsFirstToken(L.LastToken)) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"except assign stmt FIRST");
        }

        if (! IsAssignStmt(L)) {
            return NULL; //不是Assign Stmt
        }

        AccessExpressionNode *Access = AccessExpressionNode::Parser::INSTANCE.Parse(L);

        bool Flag;
        switch (L.LastToken.Kind) {
        case Lexer::TokenKind::AssignSign: case Lexer::TokenKind::RemainderAssignment:
        case Lexer::TokenKind::AdditionAssignment: case Lexer::TokenKind::SubtractionAssignment:
        case Lexer::TokenKind::MultiplicationAssignment: case Lexer::TokenKind::DivisionAssignment:
            Flag = true;
            break;
        default:
            Flag = false;
            break;
        }

        if (! Flag) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"except assign operator");
        }
        
        Lexer::Token AssignOperator = L.LastToken;
        L.Scan();

        ExpressionNode *Expression = ExpressionNode::Parser::INSTANCE.Parse(L);

        if (L.LastToken.Kind != Lexer::TokenKind::Semicolon) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"except a semicolon ';'!");
        }

        Lexer::Token Semicolon = L.LastToken;
        L.Scan();

        AssignStmtNode *Result = new AssignStmtNode(Access, AssignOperator, Expression, Semicolon);
        return Result;
    }
}