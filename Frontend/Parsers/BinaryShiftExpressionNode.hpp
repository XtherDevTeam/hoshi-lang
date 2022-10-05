//
// Created by chou on 2022/8/29.
//

#ifndef XSCRIPT2_BINARYSHIFTEXPRESSIONNODE_HPP
#define XSCRIPT2_BINARYSHIFTEXPRESSIONNODE_HPP

#include <Parsers/AdditionExpressionNode.hpp>

namespace Hoshi {
    class BinaryShiftExpressionNode : public CSTNode {
        XArray<AdditionExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;

        BinaryShiftExpressionNode(XArray<AdditionExpressionNode *> Operands, XArray<Lexer::Token> Operators);

    public:
        BinaryShiftExpressionNode();

        ~BinaryShiftExpressionNode() override;

        XArray<AdditionExpressionNode *> GetOperands();

        AdditionExpressionNode *GetOperands(int index);

        XArray<Lexer::Token> GetOperators();

        Lexer::Token GetOperators(int index);

        XString GetNodeType() override;

#define BINARYSHIFT_FIRST ADDITION_FIRST

        class Parser : public CSTNode::Parser<BinaryShiftExpressionNode> {
            Parser();

        public:
            static Parser INSTANCE;

            BinaryShiftExpressionNode *Parse(Lexer &L) override;
        };
    };
}

#endif