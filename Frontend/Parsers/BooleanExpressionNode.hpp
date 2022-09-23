//
// Created by p0010 on 22-9-23.
//

#ifndef HOSHI_LANG_BOOLEANEXPRESSIONNODE_HPP
#define HOSHI_LANG_BOOLEANEXPRESSIONNODE_HPP

//
// Created by p0010 on 22-9-11.
//

#include "CSTNode.hpp"
#include "BinaryExpressionNode.hpp"

namespace Hoshi {

    class BooleanExpressionNode : public CSTNode {
        XArray<BinaryExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;

        BooleanExpressionNode(XArray<BinaryExpressionNode *> Operands, XArray<Lexer::Token> Operators);

    public:
        BooleanExpressionNode();

        ~BooleanExpressionNode() override;

        XArray<BinaryExpressionNode *> GetOperands();

        BinaryExpressionNode *GetOperands(int index);

        XArray<Lexer::Token> GetOperators();

        Lexer::Token GetOperators(int index);

        XString GetNodeType() override;

#define BOOLEAN_FIRST BINARY_FIRST

        class Parser : public CSTNode::Parser<BooleanExpressionNode> {
            Parser();

        public:
            static Parser INSTANCE;

            BooleanExpressionNode *Parse(Lexer &L) override;
        };
    };

} // Hoshi

#endif //HOSHI_LANG_BOOLEANEXPRESSIONNODE_HPP
