//
// Created by theflysong on 2022/8/28.
//

#include <Parsers/LiteralsNode.hpp>

namespace Hoshi {
    LiteralsNode::LiteralsNode(Lexer::Token Literals) 
        : Literals(Literals), CSTNode(Literals.Line, Literals.Column) {
    }

    LiteralsNode::LiteralsNode(void)
        : Literals({}), CSTNode(0, 0) {
    }
    LiteralsNode::~LiteralsNode() = default;

    Lexer::Token LiteralsNode::GetLiterals(void) {
        return Literals;
    }

    XString LiteralsNode::GetNodeType(void) {
        return L"literals";
    }

    LiteralsNode::Parser::Parser(void) 
        : CSTNode::Parser<LiteralsNode>({LITERALS_FIRST}) {
    }

    LiteralsNode::Parser LiteralsNode::Parser::INSTANCE;

    LiteralsNode *LiteralsNode::Parser::Parse(Lexer &L) {
        if (! IsFirstToken(L.LastToken))
            throw ParserException(L.LastToken.Line, L.LastToken.Column, L"Except literal FIRST!");
        LiteralsNode *Result = new LiteralsNode(L.LastToken);
        L.Scan();
        return Result;
    }
}