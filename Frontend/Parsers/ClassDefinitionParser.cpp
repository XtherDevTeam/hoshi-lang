//
// Created by p0010 on 22-8-25.
//

#include "ClassDefinitionParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"
#include "InnerClassDefinitionStmtParser.hpp"

namespace Hoshi {
    namespace Parser {
        ClassDefinitionParser::ClassDefinitionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST ClassDefinitionParser::Parse() {
            AST Name, Extend, Impl, Block;
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"class", 0, 0}) {
                return {};
            }
            L.Scan();
            if (L.LastToken.Kind != Lexer::TokenKind::Identifier) {
                Throw(L"ClassDefinitionParser", L"Expected a identifier");
                return {};
            }
            Name = {AST::TreeType::Identifier, L.LastToken};
            L.Scan();
            Extend = {AST::TreeType::ExtendBlock, (XArray<AST>) {}};
            if (L.LastToken == (Lexer::Token) {Lexer::TokenKind::Keywords, L"extends", 0, 0}) {
                L.Scan();
                Extend.Subtrees.push_back(StaticMemberAccessExpressionParser(L).Parse());
                if (Extend.Subtrees[0].IsNotMatchNode()) {
                    Throw(L"ClassDefinitionParser", L"Expected a class name");
                    return {};
                }
            }
            Impl = {AST::TreeType::ImplBlock, (XArray<AST>) {}};
            if (L.LastToken == (Lexer::Token) {Lexer::TokenKind::Keywords, L"impl", 0, 0}) {
                L.Scan();
                Impl.Subtrees.push_back(StaticMemberAccessExpressionParser(L).Parse());
                if (Impl.Subtrees.back().IsNotMatchNode()) {
                    Throw(L"ClassDefinitionParser", L"Expected a interface name");
                    return {};
                }
                while (L.LastToken.Kind == Lexer::TokenKind::Colon) {
                    if (Impl.Subtrees.back().IsNotMatchNode()) {
                        Throw(L"ClassDefinitionParser", L"Expected a interface name");
                        return {};
                    }
                    L.Scan();
                    Impl.Subtrees.push_back(StaticMemberAccessExpressionParser(L).Parse());
                }
                if (Impl.Subtrees.back().IsNotMatchNode()) {
                    Throw(L"ClassDefinitionParser", L"Expected a interface name");
                    return {};
                }
            }
            if (L.LastToken.Kind != Lexer::TokenKind::LeftBraces) {
                Throw(L"ClassDefinitionParser", L"Expected a `{`");
                return {};
            }
            L.Scan();

            XArray<AST> ASTs;
            AST Temp = InnerClassDefinitionStmtParser(L).Parse();
            while (!Temp.IsNotMatchNode()) {
                while (!Temp.IsNotMatchNode()) {
                    ASTs.push_back(Temp);
                    switch (L.LastToken.Kind) {
                        case Lexer::TokenKind::Semicolon: {
                            L.Scan();
                            Temp = InnerClassDefinitionStmtParser(L).Parse();
                            break;
                        }
                        case Lexer::TokenKind::RightBraces: {
                            L.Scan();
                            Block = {AST::TreeType::CodeBlock, ASTs};
                            return {AST::TreeType::ClassDefinition, {Name, Extend, Impl, Block}};
                        }
                        default: {
                            switch (Temp.Type) {
                                case AST::TreeType::FunctionDefinition:
                                    break;
                                default: {
                                    Throw(L"ClassDefinitionParser", L"Unexpected token");
                                    return {};
                                }
                            }
                        }
                    }
                }
            }
            if (L.LastToken.Kind == Lexer::TokenKind::RightBraces) {
                L.Scan();
                Block = {AST::TreeType::CodeBlock, ASTs};
                return {AST::TreeType::ClassDefinition, {Name, Extend, Impl, Block}};
            } else {
                Throw(L"ClassDefinitionParser", L"Expected `}`");
                return {};
            }
        }
    } // Hoshi
} // Parser