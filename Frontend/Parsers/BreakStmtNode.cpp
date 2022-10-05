//
// Created by XIaokang00010 on 2022/10/5.
//

#include "BreakStmtNode.hpp"

namespace Hoshi {
    BreakStmtNode::BreakStmtNode(Lexer::Token Semicolon) :
            Semicolon(std::move(Semicolon)), CSTNode(Semicolon.Line, Semicolon.Column) {

    }

    BreakStmtNode::BreakStmtNode()
            : Semicolon({}), CSTNode(0, 0) {
    }

    BreakStmtNode::~BreakStmtNode() = default;

    XString BreakStmtNode::GetNodeType() {
        return L"BreakStmt";
    }

    BreakStmtNode::Parser::Parser() : CSTNode::Parser<BreakStmtNode>({BREAK_STMT_FIRST}) {

    }

    BreakStmtNode::Parser BreakStmtNode::Parser::INSTANCE;

    BreakStmtNode *BreakStmtNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken)) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"expected break stmt FIRST");
        }
        L.Scan();

        if (L.LastToken.Kind != Lexer::TokenKind::Semicolon) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"expected a semicolon ';'!");
        }

        Lexer::Token Semicolon = L.LastToken;
        L.Scan();

        auto *Result = new BreakStmtNode(Semicolon);
        return Result;
    }
} // Hoshi