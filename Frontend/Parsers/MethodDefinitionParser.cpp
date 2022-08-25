//
// Created by p0010 on 22-8-25.
//

#include "MethodDefinitionParser.hpp"
#include "IdentifierParser.hpp"
#include "ArgumentsDeclarationParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"
#include "CodeBlockParser.hpp"
#include "FunctionInvokeExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        MethodDefinitionParser::MethodDefinitionParser(Hoshi::Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST MethodDefinitionParser::Parse() {
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"func", 0, 0}) {
                return {};
            }
            L.Scan();
            AST Name, ArgumentsDecl, InitializerList, ResultType, CodeBlock;
            Name = IdentifierParser(L).Parse();
            if (Name.IsNotMatchNode()) {
                Throw(L"MethodDefinitionParser", L"Expected a function name");
                return {};
            }
            ArgumentsDecl = ArgumentsDeclarationParser(L).Parse();
            if (ArgumentsDecl.IsNotMatchNode()) {
                Throw(L"MethodDefinitionParser", L"Expected argument declarations");
                return {};
            }
            if (L.LastToken.Kind == Lexer::TokenKind::TypeDescriptorSign) {
                L.Scan();
                XArray<AST> Initializers;
                AST Temp = FunctionInvokeExpressionParser(L).Parse();
                if (Temp.IsNotMatchNode()) {
                    Throw(L"MethodDefinitionParser", L"Expected an initializer");
                    return {};
                }
                Initializers.push_back(Temp);
                while (L.LastToken.Kind == Lexer::TokenKind::Colon) {
                    if (Temp.IsNotMatchNode()) {
                        Throw(L"MethodDefinitionParser", L"Expected an initializer");
                        return {};
                    }
                    Initializers.push_back(Temp);
                    L.Scan();
                }
                InitializerList = {AST::TreeType::InitializerList, Initializers};
            }
            if (L.LastToken.Kind != Lexer::TokenKind::ToSign) {
                Throw(L"MethodDefinitionParser", L"Expected `->`");
                return {};
            }
            L.Scan();
            ResultType = StaticMemberAccessExpressionParser(L).Parse();
            if (ResultType.IsNotMatchNode()) {
                Throw(L"MethodDefinitionParser", L"Expected result type");
                return {};
            }
            CodeBlock = CodeBlockParser(L).Parse();
            if (CodeBlock.IsNotMatchNode()) {
                Throw(L"MethodDefinitionParser", L"Expected result type");
                return {};
            }
            return {AST::TreeType::MethodDefinition, {Name, ArgumentsDecl, InitializerList, ResultType, CodeBlock}};
        }
    } // Hoshi
} // Parser