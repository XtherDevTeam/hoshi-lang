//
// Created by p0010 on 22-8-26.
//

#include "VirtualMethodDeclarationParser.hpp"
#include "IdentifierParser.hpp"
#include "ArgumentsDeclarationParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        VirtualMethodDeclarationParser::VirtualMethodDeclarationParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST VirtualMethodDeclarationParser::Parse() {
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"virtual", 0, 0}) {
                return {};
            }
            L.Scan();
            AST Name, ArgumentsDecl, ResultType;
            Name = IdentifierParser(L).Parse();
            if (Name.IsNotMatchNode()) {
                Throw(L"VirtualMethodDeclarationParser", L"Expected a method name");
                return {};
            }
            ArgumentsDecl = ArgumentsDeclarationParser(L).Parse();
            if (ArgumentsDecl.IsNotMatchNode()) {
                Throw(L"VirtualMethodDeclarationParser", L"Expected argument declarations");
                return {};
            }
            if (L.LastToken.Kind != Lexer::TokenKind::ToSign) {
                Throw(L"VirtualMethodDeclarationParser", L"Expected `->`");
                return {};
            }
            L.Scan();
            ResultType = StaticMemberAccessExpressionParser(L).Parse();
            if (ResultType.IsNotMatchNode()) {
                Throw(L"VirtualMethodDeclarationParser", L"Expected result type");
                return {};
            }
            return {AST::TreeType::VirtualMethodDeclaration, {Name, ArgumentsDecl, ResultType}};
        }
    } // Hoshi
} // Parser