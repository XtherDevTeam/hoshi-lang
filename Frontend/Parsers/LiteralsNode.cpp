//
// Created by theflysong on 2022/8/28.
//

#include <Parsers/LiteralsNode.hpp>

namespace Hoshi {
    LiteralsNode::LiteralsNode(void) = default;
    LiteralsNode::~LiteralsNode() = default;

    LiteralsNode::LiteralsNode(Lexer::Token Literals) 
        : Literals(Literals) {
    }

    Lexer::Token LiteralsNode::GetLiterals(void) {
        return Literals;
    }

    XString LiteralsNode::GetNodeType(void) {
        return L"literals";
    }

    LiteralsNode::Parser::Parser(void) 
        : CSTNode::Parser<LiteralsNode>({LITERALS_FIRST})
    {
    }

    LiteralsNode::Parser LiteralsNode::Parser::INSTANCE;

    LiteralsNode *LiteralsNode::Parser::Parse(Lexer &L) {
        if (! IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except a literal!");
        LiteralsNode *Result = new LiteralsNode(L.LastToken);
        L.Scan();
        return Result;
    }
}