//
// Created by chou on 8/30/22.
//

#ifndef HOSHI_LANG_LOGICCOMPARINGEXPRESSIONNODE_HPP
#define HOSHI_LANG_LOGICCOMPARINGEXPRESSIONNODE_HPP

#include <Parsers/BinaryShiftExpressionNode.hpp>

namespace Hoshi {

    class LogicComparingExpressionNode : public CSTNode {
        XArray<BinaryShiftExpressionNode *> Operands;
        XArray<Lexer::Token> Operators;

        LogicComparingExpressionNode(XArray<BinaryShiftExpressionNode *> Operands, XArray<Lexer::Token> Operators);

    public:
        LogicComparingExpressionNode();

        ~LogicComparingExpressionNode() override;

        XArray<BinaryShiftExpressionNode *> GetOperands();

        BinaryShiftExpressionNode *GetOperands(int index);

        XArray<Lexer::Token> GetOperators();

        Lexer::Token GetOperators(int index);

        XString GetNodeType() override;

#define LOGICCOMPARING_FIRST BINARYSHIFT_FIRST

        class Parser : public CSTNode::Parser<LogicComparingExpressionNode> {
            Parser();

        public:
            static Parser INSTANCE;

            LogicComparingExpressionNode *Parse(Lexer &L) override;
        };
    };

} // Hoshi

#endif //HOSHI_LANG_LOGICCOMPARINGEXPRESSIONNODE_HPP
