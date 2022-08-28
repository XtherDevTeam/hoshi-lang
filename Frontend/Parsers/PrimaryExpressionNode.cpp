//
// Created by theflysong on 2022/8/28.
//

#include <Parsers/PrimaryExpressionNode.hpp>

namespace Hoshi {
    PrimaryExpression::PrimaryExpression(LiteralsNode *Literals) 
        : Literals(Literals) {
    }

    PrimaryExpression::PrimaryExpression(void) 
        : Literals(NULL) {
    }

    PrimaryExpression::~PrimaryExpression() {
        if (Literals != NULL)
            delete Literals;
    }

    LiteralsNode *PrimaryExpression::GetLiterals(void) {
        return Literals;
    }

    XString PrimaryExpression::GetNodeType(void) {
        return L"primary";
    }

    PrimaryExpression::Parser::Parser(void) 
        : CSTNode::Parser<PrimaryExpression>({PRIMARY_FIRST})
    {
    }

    PrimaryExpression::Parser PrimaryExpression::Parser::INSTANCE;

    PrimaryExpression *PrimaryExpression::Parser::Parse(Lexer &L) {
        if (! IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except a literal!");

        if (LiteralsNode::Parser::INSTANCE.IsFirstToken(L.LastToken)) {
            LiteralsNode *Literals = LiteralsNode::Parser::INSTANCE.Parse(L);
            PrimaryExpression *Primary = new PrimaryExpression(Literals);
            return Primary;
        }

        return NULL;
    }
}