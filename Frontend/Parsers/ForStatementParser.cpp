//
// Created by p0010 on 22-8-24.
//

#include "ForStatementParser.hpp"
#include "StatementParser.hpp"
#include "AsExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        ForStatementParser::ForStatementParser(Hoshi::Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST ForStatementParser::Parse() {
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"for", 0, 0}) {
                return {};
            }
            L.Scan();

            AST InitialStmt, Condition, AfterStmt, CodeBlock;

            if (L.LastToken.Kind != Lexer::TokenKind::LeftParentheses) {
                Throw(L"ForStatementParser", L"Expected a `(`");
                return {};
            }
            L.Scan();

            InitialStmt = StatementParser(L).Parse();
            if (InitialStmt.IsNotMatchNode()) {
                Throw(L"ForStatementParser", L"Expected a statement");
                return {};
            }
            if (L.LastToken.Kind != Lexer::TokenKind::Semicolon) {
                Throw(L"ForStatementParser", L"Expected a `;`");
                return {};
            }
            L.Scan();

            Condition = AsExpressionParser(L).Parse();
            if (Condition.IsNotMatchNode()) {
                Throw(L"ForStatementParser", L"Expected a condition");
                return {};
            }
            if (L.LastToken.Kind != Lexer::TokenKind::Semicolon) {
                Throw(L"ForStatementParser", L"Expected a `;`");
                return {};
            }
            L.Scan();

            AfterStmt = StatementParser(L).Parse();
            if (AfterStmt.IsNotMatchNode()) {
                Throw(L"ForStatementParser", L"Expected a statement");
                return {};
            }
            if (L.LastToken.Kind != Lexer::TokenKind::RightParentheses) {
                Throw(L"ForStatementParser", L"Expected a `)`");
                return {};
            }
            L.Scan();

            CodeBlock = StatementParser(L).Parse();
            if (CodeBlock.IsNotMatchNode()) {
                Throw(L"ForStatementParser", L"Expected a statement");
                return {};
            }
            return {AST::TreeType::ForStatement, {InitialStmt, Condition, AfterStmt, CodeBlock}};
        }
    } // Hoshi
} // Parser