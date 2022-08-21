//
// Created by p0010 on 22-8-21.
//

#include "IdentifierParser.hpp"

namespace Hoshi {
    namespace Parser {
        IdentifierParser::IdentifierParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST IdentifierParser::Parse() {
            if (L.LastToken.Kind == Lexer::TokenKind::Identifier) {
                AST Res{AST::TreeType::Identifier, L.LastToken};
                L.Scan();
                return Res;
            }
            Rollback();
            return {};
        }
    } // Hoshi
} // Parser