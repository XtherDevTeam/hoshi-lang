//
// Created by p0010 on 22-8-24.
//

#include "IfStatementParser.hpp"
#include "CodeBlockParser.hpp"
#include "StatementParser.hpp"
#include "PrimaryParser.hpp"

namespace Hoshi {
    namespace Parser {
        IfStatementParser::IfStatementParser(Hoshi::Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST IfStatementParser::Parse() {
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"if", 0, 0}) {
                return {};
            }
            L.Scan();
            AST Condition, ThenBlock, ElseBlock;

            Condition = PrimaryParser(L).Parse();
            if (Condition.IsNotMatchNode()) {
                Throw(L"IfStatementParser", L"Expected a condition");
                return {};
            }

            ThenBlock = CodeBlockParser(L).Parse();
            if (ThenBlock.IsNotMatchNode()) {
                Throw(L"IfStatementParser", L"Expected a ThenBlock");
                return {};
            }
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"else", 0, 0}) {
                return {AST::TreeType::IfStatement, {ThenBlock}};
            }
            L.Scan();
            ElseBlock = StatementParser(L).Parse();
            if (ThenBlock.IsNotMatchNode()) {
                Throw(L"IfStatementParser", L"Expected a ElseBlock");
                return {};
            }
            return {AST::TreeType::IfElseStatement, {ThenBlock, ElseBlock}};
        }
    } // Hoshi
} // Parser