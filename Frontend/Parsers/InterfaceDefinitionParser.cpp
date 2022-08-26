//
// Created by p0010 on 22-8-26.
//

#include "InterfaceDefinitionParser.hpp"
#include "IdentifierParser.hpp"
#include "VirtualMethodDeclarationParser.hpp"

namespace Hoshi {
    namespace Parser {
        InterfaceDefinitionParser::InterfaceDefinitionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST InterfaceDefinitionParser::Parse() {
            if (L.LastToken != (Lexer::Token){Lexer::TokenKind::Keywords, L"interface", 0 , 0}) {
                return {};
            }
            L.Scan();
            AST Identifier = IdentifierParser(L).Parse(), Block;
            if (Identifier.IsNotMatchNode()) {
                Throw(L"InterfaceDefinitionParser", L"Expected a identifier");
                return {};
            }
            if (L.LastToken.Kind != Lexer::TokenKind::LeftBraces) {
                Throw(L"InterfaceDefinitionParser", L"Expected a `{`");
                return {};
            }
            L.Scan();
            XArray<AST> ASTs;
            AST Temp = VirtualMethodDeclarationParser(L).Parse();
            while (!Temp.IsNotMatchNode()) {
                while (!Temp.IsNotMatchNode()) {
                    ASTs.push_back(Temp);
                    switch (L.LastToken.Kind) {
                        case Lexer::TokenKind::Semicolon: {
                            L.Scan();
                            Temp = VirtualMethodDeclarationParser(L).Parse();
                            break;
                        }
                        case Lexer::TokenKind::RightBraces: {
                            L.Scan();
                            Block = {AST::TreeType::CodeBlock, ASTs};
                            return {AST::TreeType::InterfaceDefinition, {Identifier, Block}};
                        }
                        default: {
                            switch (Temp.Type) {
                                case AST::TreeType::FunctionDefinition:
                                    break;
                                default: {
                                    Throw(L"InterfaceDefinitionParser", L"Unexpected token");
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
                return {AST::TreeType::InterfaceDefinition, {Identifier, Block}};
            } else {
                Throw(L"InterfaceDefinitionParser", L"Expected a `}`");
                return {};
            }
        }
    } // Hoshi
} // Parser