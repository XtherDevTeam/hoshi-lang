//
// Created by p0010 on 22-8-27.
//

#include "ImportStatementParser.hpp"

namespace Hoshi {
    namespace Parser {
        ImportStatementParser::ImportStatementParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST ImportStatementParser::Parse() {
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"import", 0, 0}) {
                return {};
            }
            L.Scan();
            AST ModuleName, ModulePath;
            if (L.LastToken.Kind != Lexer::TokenKind::String) {
                Throw(L"ImportStatementParser", L"Expected a string");
                return {};
            }
            ModulePath = {AST::TreeType::Literals, L.LastToken};
            L.Scan();
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"as", 0, 0}) {
                Throw(L"ImportStatementParser", L"Expected `as`");
                return {};
            }
            L.Scan();
            if (L.LastToken.Kind != Lexer::TokenKind::Identifier) {
                Throw(L"ImportStatementParser", L"Expected an identifier");
                return {};
            }
            ModuleName = {AST::TreeType::Identifier, L.LastToken};
            L.Scan();
            return {AST::TreeType::ImportStatement, {ModulePath, ModuleName}};
        }
    } // Hoshi
} // Parser