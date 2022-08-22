//
// Created by Jerry Chou on 22-8-22.
//

#include "LiteralsParser.hpp"

namespace Hoshi {
    namespace Parser {
        LiteralsParser::LiteralsParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST LiteralsParser::Parse() {
            switch (L.LastToken.Kind) {
                case Lexer::TokenKind::Integer:
                case Lexer::TokenKind::Boolean:
                case Lexer::TokenKind::Decimal:
                case Lexer::TokenKind::String:
                case Lexer::TokenKind::Character: {
                    AST Res{AST::TreeType::Literals, L.LastToken};
                    L.Scan();
                    return Res;
                }
                default: {
                    Rollback();
                    return {};
                }
            }
        }
    } // Hoshi
} // Parser