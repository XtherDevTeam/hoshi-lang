//
// Created by XIaokang00010 on 2022/10/5.
//

#include "ContinueStmtNode.hpp"

namespace Hoshi {
    ContinueStmtNode::ContinueStmtNode(Lexer::Token Semicolon) :
            Semicolon(std::move(Semicolon)), CSTNode(Semicolon.Line, Semicolon.Column) {

    }

    ContinueStmtNode::ContinueStmtNode()
            : Semicolon({}), CSTNode(0, 0) {
    }

    ContinueStmtNode::~ContinueStmtNode() = default;

    XString ContinueStmtNode::GetNodeType() {
        return L"ContinueStmt";
    }

    ContinueStmtNode::Parser::Parser() : CSTNode::Parser<ContinueStmtNode>({CONTINUE_STMT_FIRST}) {

    }

    ContinueStmtNode::Parser ContinueStmtNode::Parser::INSTANCE;

    ContinueStmtNode *ContinueStmtNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken)) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"expected continue stmt FIRST");
        }
        L.Scan();

        if (L.LastToken.Kind != Lexer::TokenKind::Semicolon) {
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"expected a semicolon ';'!");
        }

        Lexer::Token Semicolon = L.LastToken;
        L.Scan();

        auto *Result = new ContinueStmtNode(Semicolon);
        return Result;
    }
} // Hoshi