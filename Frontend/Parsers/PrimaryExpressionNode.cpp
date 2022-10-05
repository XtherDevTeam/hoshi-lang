//
// Created by theflysong on 2022/8/28.
//

#include <Parsers/PrimaryExpressionNode.hpp>

namespace Hoshi {
    PrimaryExpressionNode::PrimaryExpressionNode(LiteralsNode *Literals)
            : Literals(Literals), Access(nullptr), CSTNode(Literals->Line, Literals->Column) {
    }

    PrimaryExpressionNode::PrimaryExpressionNode(AccessExpressionNode *Access)
            : Literals(nullptr), Access(Access), CSTNode(Access->Line, Access->Column) {
    }

    PrimaryExpressionNode::PrimaryExpressionNode()
            : Literals(nullptr), Access(nullptr), CSTNode(0, 0) {
    }

    PrimaryExpressionNode::~PrimaryExpressionNode() {
        if (Literals != nullptr)
            delete Literals;
    }

    LiteralsNode *PrimaryExpressionNode::GetLiterals() {
        return Literals;
    }

    AccessExpressionNode *PrimaryExpressionNode::GetAccess() {
        return Access;
    }

    XString PrimaryExpressionNode::GetNodeType() {
        return L"primary";
    }

    PrimaryExpressionNode::Parser::Parser()
            : CSTNode::Parser<PrimaryExpressionNode>({PRIMARY_FIRST}) {
    }

    PrimaryExpressionNode::Parser PrimaryExpressionNode::Parser::INSTANCE;

    PrimaryExpressionNode *PrimaryExpressionNode::Parser::Parse(Lexer &L) {
        if (!IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except primary expression FIRST!");

        if (LiteralsNode::Parser::INSTANCE.IsFirstToken(L.LastToken)) {
            LiteralsNode *Literals = LiteralsNode::Parser::INSTANCE.Parse(L);
            auto *Primary = new PrimaryExpressionNode(Literals);
            return Primary;
        }

        if (AccessExpressionNode::Parser::INSTANCE.IsFirstToken(L.LastToken)) {
            AccessExpressionNode *Access = AccessExpressionNode::Parser::INSTANCE.Parse(L);
            auto *Primary = new PrimaryExpressionNode(Access);
            return Primary;
        }

        return nullptr;
    }
}