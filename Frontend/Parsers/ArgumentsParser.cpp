//
// Created by p0010 on 22-8-22.
//

#include "ArgumentsParser.hpp"
#include "AsExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        ArgumentsParser::ArgumentsParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST ArgumentsParser::Parse() {
            if (L.LastToken.Kind != Lexer::TokenKind::LeftParentheses) {
                return {};
            }
            L.Scan();
            XArray<AST> ASTs;
            AST Temp = AsExpressionParser(L).Parse();
            while (!Temp.IsNotMatchNode()) {
                ASTs.push_back(Temp);
                switch (L.LastToken.Kind) {
                    case Lexer::TokenKind::Colon: {
                        L.Scan();
                        Temp = AsExpressionParser(L).Parse();
                        break;
                    }
                    case Lexer::TokenKind::RightParentheses: {
                        L.Scan();
                        return {AST::TreeType::Arguments, ASTs};
                    }
                    default: {
                        Throw(L"TemplateArgumentsParser", L"Unexpected token");
                        return {};
                    }
                }
            }
            Throw(L"ArgumentsParser", L"Expected a `)` to close a template arguments list");
            return {};
        }
    } // Hoshi
} // Parser