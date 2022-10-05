//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_MULIPLICATIONEXPRESSIONNODE_HPP
#define XSCRIPT2_MULIPLICATIONEXPRESSIONNODE_HPP

#include <Parsers/UniqueExpressionNode.hpp>

namespace Hoshi {
    class MuliplicationExpressionNode : public CSTNode {
        XArray<UniqueExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;

        MuliplicationExpressionNode(XArray<UniqueExpressionNode *> Operands, XArray<Lexer::Token> Operators);

    public:
        MuliplicationExpressionNode();

        ~MuliplicationExpressionNode() override;

        XArray<UniqueExpressionNode *> GetOperands();

        UniqueExpressionNode *GetOperands(int index);

        XArray<Lexer::Token> GetOperators();

        Lexer::Token GetOperators(int index);

        XString GetNodeType() override;

#define MULIPLICATION_FIRST UNIQUE_FIRST

        class Parser : public CSTNode::Parser<MuliplicationExpressionNode> {
            Parser();

        public:
            static Parser INSTANCE;

            MuliplicationExpressionNode *Parse(Lexer &L) override;
        };
    };
}

#endif