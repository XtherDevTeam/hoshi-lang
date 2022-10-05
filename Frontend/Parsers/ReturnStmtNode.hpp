//
// Created by XIaokang00010 on 2022/10/5.
//

#ifndef HOSHI_LANG_RETURNSTMTNODE_HPP
#define HOSHI_LANG_RETURNSTMTNODE_HPP

#include "CSTNode.hpp"
#include "ExpressionNode.hpp"

namespace Hoshi {

    class ReturnStmtNode : public CSTNode {
        ExpressionNode *Expr;
        Lexer::Token Semicolon;

        ReturnStmtNode(ExpressionNode *Expr, Lexer::Token Semicolon);
    public:
        ReturnStmtNode();

        virtual ~ReturnStmtNode();

        ExpressionNode *GetExpression();

        XString GetNodeType() override;

#define RETURN_STMT_FIRST Hoshi::Lexer::TokenKind::Return

        class Parser : public CSTNode::Parser<ReturnStmtNode> {
            Parser();

        public:
            static Parser INSTANCE;

            ReturnStmtNode *Parse(Lexer &L) override;
        };
    };

} // Hoshi

#endif //HOSHI_LANG_RETURNSTMTNODE_HPP
