//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_LITERALSNODE_HPP
#define XSCRIPT2_LITERALSNODE_HPP

#include <Parsers/CSTNode.hpp>

namespace Hoshi {
    class LiteralsNode : public CSTNode{
        Lexer::Token Literals;

        LiteralsNode(Lexer::Token Literals);
    public:
        LiteralsNode(void);

        Lexer::Token GetLiterals(void);

        virtual XString GetNodeType(void) override;

        class Parser : CSTNode::Parser<LiteralsNode> {
            Parser(void);
        public:
            static Parser INSTANCE;
            
            virtual LiteralsNode Parse(Lexer L) override;
        };
    };
}

#endif