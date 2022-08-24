//
// Created by p0010 on 22-8-24.
//

#include "WhileStatementParser.hpp"
#include "PrimaryParser.hpp"
#include "StatementParser.hpp"

namespace Hoshi {
    namespace Parser {

        WhileStatementParser::WhileStatementParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST WhileStatementParser::Parse() {
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"while", 0, 0}) {
                return {};
            }
            L.Scan();
            AST Condition, ThenBlock;

            Condition = PrimaryParser(L).Parse();
            if (Condition.IsNotMatchNode()) {
                Throw(L"WhileStatementParser", L"Expected a condition");
                return {};
            }

            ThenBlock = StatementParser(L).Parse();
            if (ThenBlock.IsNotMatchNode()) {
                Throw(L"WhileStatementParser", L"Expected a ThenBlock");
                return {};
            }

            return {AST::TreeType::WhileStatement, {Condition, ThenBlock}};
        }
    } // Parser
} // Hoshi