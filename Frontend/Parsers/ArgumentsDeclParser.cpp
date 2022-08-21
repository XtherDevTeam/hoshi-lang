//
// Created by p0010 on 22-8-21.
//

#include "ArgumentsDeclParser.hpp"
#include "IdentifierWithTypeDescriptorParser.hpp"

namespace Hoshi {
    namespace Parser {
        ArgumentsDeclParser::ArgumentsDeclParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST ArgumentsDeclParser::Parse() {
            if (L.LastToken.Kind != Lexer::TokenKind::LeftParentheses) {
                Throw(L"ArgumentsDeclParser", L"Expected a `(`");
                return {};
            }
            L.Scan();
            XArray<AST> ASTs;
            AST Temp = IdentifierWithTypeDescriptorParser(L).Parse();
            while (!Temp.IsNotMatchNode()) {
                ASTs.push_back(Temp);
                Temp = IdentifierWithTypeDescriptorParser(L).Parse();
            }

            if (L.LastToken.Kind != Lexer::TokenKind::RightParentheses) {
                Throw(L"ArgumentsDeclParser", L"Expected a `)`");
                return {};
            }

            return {AST::TreeType::ArgumentsDecl, ASTs};
        }
    } // Hoshi
} // Parser