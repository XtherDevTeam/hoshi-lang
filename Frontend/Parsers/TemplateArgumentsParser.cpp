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
                Rollback();
                return {};
            }
            L.Scan();

            XArray<AST> ASTs;
            AST Temp;
            if ((Temp = StaticMemberAccessExpressionParser(L).Parse()).IsNotMatchNode()) {
                Throw(L"TemplateArgumentsParser", L"Expected a StaticMemberAccessExpression");
            }
            while (!Temp.IsNotMatchNode()) {
                if (L.LastToken.Kind == Lexer::TokenKind::MoreThan) {
                    break;
                }
                else if (L.LastToken.Kind == Lexer::TokenKind::Colon) {
                    L.Scan();
                } else {
                    Throw(L"TemplateArgumentsParser", L"Expected a `,`");
                    return {};
                }
                ASTs.push_back(Temp);
                Temp = StaticMemberAccessExpressionParser(L).Parse();
            }
            if (L.LastToken.Kind != Lexer::TokenKind::MoreThan) {
                Throw(L"TemplateArgumentsParser", L"Expected a `>`");
            }
            L.Scan();
            return {AST::TreeType::TemplateArguments, ASTs};
        }
    } // Hoshi
} // Parser