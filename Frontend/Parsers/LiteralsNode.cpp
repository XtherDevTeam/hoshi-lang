//
// Created by theflysong on 2022/8/28.
//

#include <Parsers/LiteralsNode.hpp>

namespace Hoshi {
    LiteralsNode::LiteralsNode(Lexer::Token Literals) 
        : Literals(Literals), CSTNode(Literals.Line, Literals.Column) {
    }

    LiteralsNode::LiteralsNode()
            : Literals({}), CSTNode(0, 0) {
    }
    LiteralsNode::~LiteralsNode() = default;

    Lexer::Token LiteralsNode::GetLiterals() {
        return Literals;
    }

    XString LiteralsNode::GetNodeType() {
        return L"literals";
    }

    LiteralsNode::Parser::Parser()
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