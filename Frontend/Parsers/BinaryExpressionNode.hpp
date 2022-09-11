//
// Created by p0010 on 22-9-11.
//

#ifndef HOSHI_LANG_BINARYEXPRESSIONNODE_HPP
#define HOSHI_LANG_BINARYEXPRESSIONNODE_HPP

#include "CSTNode.hpp"
#include "LogicEqualExpressionNode.hpp"

namespace Hoshi {

    class BinaryExpressionNode : public CSTNode {
        XArray<LogicEqualExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;

        BinaryExpressionNode(XArray<LogicEqualExpressionNode *> Operands, XArray<Lexer::Token> Operators);

    public:
        BinaryExpressionNode();

        ~BinaryExpressionNode() override;

        XArray<LogicEqualExpressionNode *> GetOperands();

        LogicEqualExpressionNode *GetOperands(int index);

        XArray<Lexer::Token> GetOperators();

        Lexer::Token GetOperators(int index);

        XString GetNodeType() override;

#define BINARY_FIRST LOGICEQUAL_FIRST

        class Parser : public CSTNode::Parser<BinaryExpressionNode> {
            Parser();

        public:
            static Parser INSTANCE;

            BinaryExpressionNode *Parse(Lexer &L) override;
        };
    };

} // Hoshi


#endif //HOSHI_LANG_BINARYEXPRESSIONNODE_HPP
