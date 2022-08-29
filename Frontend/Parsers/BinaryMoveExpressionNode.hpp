//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_BINARYMOVEEXPRESSIONNODE_HPP
#define XSCRIPT2_BINARYMOVEEXPRESSIONNODE_HPP

#include <Parsers/AdditionExpressionNode.hpp>

namespace Hoshi {
    class BinaryMoveExpressionNode : public CSTNode{
        XArray<AdditionExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;

        BinaryMoveExpressionNode(XArray<AdditionExpressionNode *> Operands, XArray<Lexer::Token> Operators);
    public:
        BinaryMoveExpressionNode(void);
        virtual ~BinaryMoveExpressionNode();

        XArray<AdditionExpressionNode *> GetOperands(void);
        AdditionExpressionNode *GetOperands(int index);
        XArray<Lexer::Token> GetOperators(void);
        Lexer::Token GetOperators(int index);

        virtual XString GetNodeType(void) override;

#define BINARYMOVE_FIRST ADDITION_FIRST

        class Parser : public CSTNode::Parser<BinaryMoveExpressionNode> {
            Parser(void);
        public:
            static Parser INSTANCE;

            BinaryMoveExpressionNode *Parse(Lexer &L) override;
        };
    };
}

#endif