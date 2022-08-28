//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_PRIMARYEXPRESSIONNODE_HPP
#define XSCRIPT2_PRIMARYEXPRESSIONNODE_HPP

#include <Parsers/LiteralsNode.hpp>

namespace Hoshi {
    class PrimaryExpression : public CSTNode{
        LiteralsNode *Literals;

        PrimaryExpression(LiteralsNode *Literals);
    public:
        PrimaryExpression(void);
        virtual ~PrimaryExpression();

        LiteralsNode *GetLiterals(void);

        virtual XString GetNodeType(void) override;

        #define PRIMARY_FIRST LITERALS_FIRST

        class Parser : public CSTNode::Parser<PrimaryExpression> {
            Parser(void);
        public:
            static Parser INSTANCE;
            
            virtual PrimaryExpression *Parse(Lexer &L) override;
        };
    };
}

#endif