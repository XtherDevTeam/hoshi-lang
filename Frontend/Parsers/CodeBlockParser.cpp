//
// Created by p0010 on 22-8-23.
//

#include "CodeBlockParser.hpp"
#include "StatementParser.hpp"

namespace Hoshi {
    namespace Parser {
        CodeBlockParser::CodeBlockParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST CodeBlockParser::Parse() {
            if (L.LastToken.Kind != Lexer::TokenKind::LeftBraces) {
                return {};
            }
            L.Scan();
            XArray<AST> ASTs;
            AST Temp = StatementParser(L).Parse();
            while (!Temp.IsNotMatchNode()) {
                ASTs.push_back(Temp);
                switch (L.LastToken.Kind) {
                    case Lexer::TokenKind::Colon: {
                        L.Scan();
                        Temp = StatementParser(L).Parse();
                        break;
                    }
                    case Lexer::TokenKind::RightBraces: {
                        L.Scan();
                        return {AST::TreeType::CodeBlock, ASTs};
                    }
                    default: {
                        Throw(L"CodeBlockParser", L"Unexpected token");
                        return {};
                    }
                }
            }
            if (L.LastToken.Kind == Lexer::TokenKind::RightBraces) {
                L.Scan();
                return {AST::TreeType::CodeBlock, (XArray<AST>){}};
            }
            Throw(L"CodeBlockParser", L"Expected a `}` to close a code block");
            return {};
        }
    } // Hoshi
} // Parser