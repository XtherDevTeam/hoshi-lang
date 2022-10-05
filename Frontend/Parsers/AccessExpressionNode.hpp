//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_MEMBERACCESSEXPRESSIONNODE_HPP
#define XSCRIPT2_MEMBERACCESSEXPRESSIONNODE_HPP

#include <Parsers/CSTNode.hpp>

namespace Hoshi {
    class AccessExpressionNode : public CSTNode {
    public:
        struct AccessOperator {
            //.xxx
            Lexer::Token Dot;
            Lexer::Token Identifier;
            // //( xxx )
            // Lexer::Token LeftParentheses;
            // ArgumentsNode Arguments;
            // Lexer::Token RightParentheses;
            // //[ xxx ]
            // Lexer::Token LeftBracket;
            // ExpressionNode Expression;
            // Lexer::Token RightBracket;
        };
    private:
        Lexer::Token Identifier;
        XArray<AccessOperator> Accesses;

        AccessExpressionNode(const Lexer::Token& Identifier, XArray<AccessOperator> Accesses);

    public:
        AccessExpressionNode();

        ~AccessExpressionNode() override;

        XArray<AccessOperator> GetAccesses();

        AccessOperator GetAccesses(int index);

        Lexer::Token GetIdentifier();

        XString GetNodeType() override;

#define ACCESS_FIRST Lexer::TokenKind::Identifier

        class Parser : public CSTNode::Parser<AccessExpressionNode> {
            Parser();

        public:
            static Parser INSTANCE;

            AccessExpressionNode *Parse(Lexer &L) override;
        };
    };
}

#endif