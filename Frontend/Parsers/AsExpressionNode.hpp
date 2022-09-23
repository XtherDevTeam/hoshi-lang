//
// Created by p0010 on 22-9-23.
//

#ifndef HOSHI_LANG_ASEXPRESSIONNODE_HPP
#define HOSHI_LANG_ASEXPRESSIONNODE_HPP

#include "BooleanExpressionNode.hpp"

namespace Hoshi {

    class AsExpressionNode : public CSTNode {
        XArray<BooleanExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;

        AsExpressionNode(XArray<BooleanExpressionNode *> Operands, XArray<Lexer::Token> Operators);

    public:
        AsExpressionNode();

        ~AsExpressionNode() override;

        XArray<BooleanExpressionNode *> GetOperands();

        BooleanExpressionNode *GetOperands(int index);

        XArray<Lexer::Token> GetOperators();

        Lexer::Token GetOperators(int index);

        XString GetNodeType() override;

#define AS_FIRST BOOLEAN_FIRST

        class Parser : public CSTNode::Parser<AsExpressionNode> {
            Parser();

        public:
            static Parser INSTANCE;

            AsExpressionNode *Parse(Lexer &L) override;
        };
    };

} // Hoshi

#endif //HOSHI_LANG_ASEXPRESSIONNODE_HPP
