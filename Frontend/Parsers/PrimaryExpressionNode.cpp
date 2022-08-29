//
// Created by theflysong on 2022/8/28.
//

#include <Parsers/PrimaryExpressionNode.hpp>

namespace Hoshi {
    PrimaryExpressionNode::PrimaryExpressionNode(LiteralsNode *Literals) 
        : Literals(Literals), CSTNode(Literals->Line, Literals->Column) {
    }

    PrimaryExpressionNode::PrimaryExpressionNode(void) 
        : Literals(NULL), CSTNode(0, 0) {
    }

    PrimaryExpressionNode::~PrimaryExpressionNode() {
        if (Literals != NULL)
            delete Literals;
    }

    LiteralsNode *PrimaryExpressionNode::GetLiterals(void) {
        return Literals;
    }

    XString PrimaryExpressionNode::GetNodeType(void) {
        return L"primary";
    }

    PrimaryExpressionNode::Parser::Parser(void) 
        : CSTNode::Parser<PrimaryExpressionNode>({PRIMARY_FIRST}) {
    }

    PrimaryExpressionNode::Parser PrimaryExpressionNode::Parser::INSTANCE;

    PrimaryExpressionNode *PrimaryExpressionNode::Parser::Parse(Lexer &L) {
        if (! IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except primary expression FIRST!");

        if (LiteralsNode::Parser::INSTANCE.IsFirstToken(L.LastToken)) {
            LiteralsNode *Literals = LiteralsNode::Parser::INSTANCE.Parse(L);
            PrimaryExpressionNode *Primary = new PrimaryExpressionNode(Literals);
            return Primary;
        }

        return NULL;
    }
}