//
// Created by XIaokang00010 on 2022/10/5.
//

#ifndef HOSHI_LANG_BREAKSTMTNODE_HPP
#define HOSHI_LANG_BREAKSTMTNODE_HPP

#include "CSTNode.hpp"

namespace Hoshi {

    class BreakStmtNode : public CSTNode {
        Lexer::Token Semicolon;

        BreakStmtNode(Lexer::Token Semicolon);
    public:
        BreakStmtNode();

        virtual ~BreakStmtNode();

        XString GetNodeType() override;

#define BREAK_STMT_FIRST Hoshi::Lexer::TokenKind::Break

        class Parser : public CSTNode::Parser<BreakStmtNode> {
            Parser();

        public:
            static Parser INSTANCE;

            BreakStmtNode *Parse(Lexer &L) override;
        };
    };

} // Hoshi

#endif //HOSHI_LANG_BREAKSTMTNODE_HPP
