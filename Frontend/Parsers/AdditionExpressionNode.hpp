//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_ADDITIONEXPRESSIONNODE_HPP
#define XSCRIPT2_ADDITIONEXPRESSIONNODE_HPP

#include <Parsers/MuliplicationExpressionNode.hpp>

namespace Hoshi {
    class AdditionExpressionNode : public CSTNode{
        XArray<MuliplicationExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;

        AdditionExpressionNode(XArray<MuliplicationExpressionNode *> Operands, XArray<Lexer::Token> Operators);
    public:
        AdditionExpressionNode(void);
        virtual ~AdditionExpressionNode();

        XArray<MuliplicationExpressionNode *> GetOperands(void);
        MuliplicationExpressionNode *GetOperands(int index);
        XArray<Lexer::Token> GetOperators(void);
        Lexer::Token GetOperators(int index);

        virtual XString GetNodeType(void) override;

        #define ADDITION_FIRST MULIPLICATION_FIRST

        class Parser : public CSTNode::Parser<AdditionExpressionNode> {
            Parser(void);
        public:
            static Parser INSTANCE;
            
            virtual AdditionExpressionNode *Parse(Lexer &L) override;
        };
    };
}

#endif