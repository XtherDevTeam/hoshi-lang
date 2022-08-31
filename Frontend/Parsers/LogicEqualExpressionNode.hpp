//
// Created by chou on 8/31/22.
//

#ifndef HOSHI_LANG_LOGICEQUALEXPRESSIONNODE_HPP
#define HOSHI_LANG_LOGICEQUALEXPRESSIONNODE_HPP

#include "CSTNode.hpp"
#include "LogicComparingExpressionNode.hpp"

namespace Hoshi {

    class LogicEqualExpressionNode : public CSTNode {
        XArray<LogicComparingExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;

        LogicEqualExpressionNode(XArray<LogicComparingExpressionNode *> Operands, XArray<Lexer::Token> Operators);

    public:
        LogicEqualExpressionNode();

        ~LogicEqualExpressionNode() override;

        XArray<LogicComparingExpressionNode *> GetOperands();

        LogicComparingExpressionNode *GetOperands(int index);

        XArray<Lexer::Token> GetOperators();

        Lexer::Token GetOperators(int index);

        XString GetNodeType() override;

#define LOGICEQUAL_FIRST LOGICCOMPARING_FIRST

        class Parser : public CSTNode::Parser<LogicEqualExpressionNode> {
            Parser();

        public:
            static Parser INSTANCE;

            LogicEqualExpressionNode *Parse(Lexer &L) override;
        };
    };

} // Hoshi

#endif //HOSHI_LANG_LOGICEQUALEXPRESSIONNODE_HPP
