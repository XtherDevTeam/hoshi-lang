//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_PRIMARYEXPRESSIONNODE_HPP
#define XSCRIPT2_PRIMARYEXPRESSIONNODE_HPP

#include <Parsers/LiteralsNode.hpp>

namespace Hoshi {
    class PrimaryExpressionNode : public CSTNode{
        LiteralsNode *Literals;

        PrimaryExpressionNode(LiteralsNode *Literals);
    public:
        PrimaryExpressionNode(void);
        virtual ~PrimaryExpressionNode();

        LiteralsNode *GetLiterals(void);

        virtual XString GetNodeType(void) override;

        #define PRIMARY_FIRST LITERALS_FIRST

        class Parser : public CSTNode::Parser<PrimaryExpressionNode> {
            Parser(void);
        public:
            static Parser INSTANCE;
            
            virtual PrimaryExpressionNode *Parse(Lexer &L) override;
        };
    };
}

#endif