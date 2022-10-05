//
// Created by XIaokang00010 on 2022/10/5.
//

#ifndef HOSHI_LANG_CONTINUESTMTNODE_HPP
#define HOSHI_LANG_CONTINUESTMTNODE_HPP

#include "CSTNode.hpp"

namespace Hoshi {

    class ContinueStmtNode : public CSTNode {
        Lexer::Token Semicolon;

        ContinueStmtNode(Lexer::Token Semicolon);
    public:
        ContinueStmtNode();

        virtual ~ContinueStmtNode();

        XString GetNodeType() override;

#define CONTINUE_STMT_FIRST Hoshi::Lexer::TokenKind::Continue

        class Parser : public CSTNode::Parser<ContinueStmtNode> {
            Parser();

        public:
            static Parser INSTANCE;

            ContinueStmtNode *Parse(Lexer &L) override;
        };
    };

} // Hoshi

#endif //HOSHI_LANG_CONTINUESTMTNODE_HPP
