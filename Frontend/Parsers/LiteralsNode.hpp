//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_LITERALSNODE_HPP
#define XSCRIPT2_LITERALSNODE_HPP

#include <Parsers/CSTNode.hpp>

namespace Hoshi {
    class LiteralsNode : public CSTNode {
        Lexer::Token Literals;

        LiteralsNode(Lexer::Token Literals);
    public:
        LiteralsNode();

        virtual ~LiteralsNode();

        Lexer::Token GetLiterals();

        XString GetNodeType() override;

        #define LITERALS_FIRST Lexer::TokenKind::Integer, \
            Lexer::TokenKind::Boolean, \
            Lexer::TokenKind::Decimal, \
            Lexer::TokenKind::String, \
            Lexer::TokenKind::Character

        class Parser : public CSTNode::Parser<LiteralsNode> {
            Parser();
        public:
            static Parser INSTANCE;

            virtual LiteralsNode *Parse(Lexer &L) override;
        };
    };
}

#endif