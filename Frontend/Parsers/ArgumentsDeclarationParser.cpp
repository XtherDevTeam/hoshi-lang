//
// Created by p0010 on 22-8-23.
//

#include "ArgumentsDeclarationParser.hpp"
#include "IdentifierWithTypeDescriptorParser.hpp"

namespace Hoshi {
    namespace Parser {
        ArgumentsDeclarationParser::ArgumentsDeclarationParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST ArgumentsDeclarationParser::Parse() {
            if (L.LastToken.Kind != Lexer::TokenKind::LeftParentheses) {
                return {};
            }
            L.Scan();
            XArray<AST> ASTs;
            AST Temp = IdentifierWithTypeDescriptorParser(L).Parse();
            while (!Temp.IsNotMatchNode()) {
                ASTs.push_back(Temp);
                switch (L.LastToken.Kind) {
                    case Lexer::TokenKind::Colon: {
                        L.Scan();
                        Temp = IdentifierWithTypeDescriptorParser(L).Parse();
                        break;
                    }
                    case Lexer::TokenKind::RightParentheses: {
                        L.Scan();
                        return {AST::TreeType::Arguments, ASTs};
                    }
                    default: {
                        Throw(L"ArgumentsDeclarationParser", L"Unexpected token");
                        return {};
                    }
                }
            }
            if (L.LastToken.Kind == Lexer::TokenKind::RightParentheses) {
                L.Scan();
                return {AST::TreeType::Arguments, (XArray<AST>){}};
            }
            Throw(L"ArgumentsDeclarationParser", L"Expected a `)` to close a template arguments list");
            return {};
        }
    } // Hoshi
} // Parser