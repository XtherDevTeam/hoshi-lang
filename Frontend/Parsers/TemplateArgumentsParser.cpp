//
// Created by p0010 on 22-8-22.
//

#include "TemplateArgumentsParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        TemplateArgumentsParser::TemplateArgumentsParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST TemplateArgumentsParser::Parse() {
            if (L.LastToken.Kind != Lexer::TokenKind::LessThan) {
                return {};
            }
            L.Scan();
            XArray<AST> ASTs;
            AST Temp = StaticMemberAccessExpressionParser(L).Parse();
            while (!Temp.IsNotMatchNode()) {
                ASTs.push_back(Temp);
                switch (L.LastToken.Kind) {
                    case Lexer::TokenKind::Colon: {
                        L.Scan();
                        Temp = StaticMemberAccessExpressionParser(L).Parse();
                        break;
                    }
                    case Lexer::TokenKind::MoreThan: {
                        L.Scan();
                        return {AST::TreeType::TemplateArguments, ASTs};
                    }
                    default: {
                        Throw(L"TemplateArgumentsParser", L"Unexpected token");
                        return {};
                    }
                }
            }
            Throw(L"TemplateArgumentsParser", L"Expected a `>` to close a template arguments list");
            return {};
        }
    } // Hoshi
} // Parser