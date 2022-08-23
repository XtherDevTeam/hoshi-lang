//
// Created by p0010 on 22-8-23.
//

#include "FunctionDefinitionParser.hpp"
#include "IdentifierParser.hpp"
#include "ArgumentsDeclarationParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"
#include "CodeBlockParser.hpp"

namespace Hoshi {
    namespace Parser {
        FunctionDefinitionParser::FunctionDefinitionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST FunctionDefinitionParser::Parse() {
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"func", 0, 0}) {
                return {};
            }
            L.Scan();
            AST Name, ArgumentsDecl, ResultType, CodeBlock;
            Name = IdentifierParser(L).Parse();
            if (Name.IsNotMatchNode()) {
                Throw(L"FunctionDefinitionParser", L"Expected a function name");
                return {};
            }
            ArgumentsDecl = ArgumentsDeclarationParser(L).Parse();
            if (ArgumentsDecl.IsNotMatchNode()) {
                Throw(L"FunctionDefinitionParser", L"Expected argument declarations");
                return {};
            }
            if (L.LastToken.Kind != Lexer::TokenKind::ToSign) {
                Throw(L"FunctionDefinitionParser", L"Expected `->`");
                return {};
            }
            L.Scan();
            ResultType = StaticMemberAccessExpressionParser(L).Parse();
            if (ResultType.IsNotMatchNode()) {
                Throw(L"FunctionDefinitionParser", L"Expected result type");
                return {};
            }
            CodeBlock = CodeBlockParser(L).Parse();
            if (CodeBlock.IsNotMatchNode()) {
                Throw(L"FunctionDefinitionParser", L"Expected result type");
                return {};
            }
            return {AST::TreeType::FunctionDefinition, {Name, ArgumentsDecl, ResultType, CodeBlock}};
        }
    } // Hoshi
} // Parser