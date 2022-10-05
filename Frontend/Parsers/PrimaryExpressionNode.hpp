//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_PRIMARYEXPRESSIONNODE_HPP
#define XSCRIPT2_PRIMARYEXPRESSIONNODE_HPP

#include <Parsers/LiteralsNode.hpp>
#include <Parsers/AccessExpressionNode.hpp>

namespace Hoshi {
    class PrimaryExpressionNode : public CSTNode {
        LiteralsNode *Literals;
        AccessExpressionNode *Access;

        PrimaryExpressionNode(LiteralsNode *Literals);

        PrimaryExpressionNode(AccessExpressionNode *Access);

    public:
        PrimaryExpressionNode();

        virtual ~PrimaryExpressionNode();

        LiteralsNode *GetLiterals();

        AccessExpressionNode *GetAccess();

        XString GetNodeType() override;

#define PRIMARY_FIRST LITERALS_FIRST, \
                              ACCESS_FIRST

        class Parser : public CSTNode::Parser<PrimaryExpressionNode> {
            Parser();

        public:
            static Parser INSTANCE;
            
            virtual PrimaryExpressionNode *Parse(Lexer &L) override;
        };
    };
}

#endif