//
// Created by p0010 on 22-8-21.
//

#include "IdentifierWithTypeDescriptorParser.hpp"
#include "IdentifierParser.hpp"

namespace Hoshi {
    namespace Parser {
        IdentifierWithTypeDescriptorParser::IdentifierWithTypeDescriptorParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST IdentifierWithTypeDescriptorParser::Parse() {
            AST Id = IdentifierParser(L).Parse();
            if (Id.IsNotMatchNode())
                return {};
            if (L.LastToken.Kind != Lexer::TokenKind::TypeDescriptorSign)
                return {};
            L.Scan();
            AST Id2 = IdentifierParser(L).Parse();
            if (Id2.IsNotMatchNode())
                Throw(L"IdentifierWithTypeDescriptorParser", L"Expected a type descriptor");
            else
                return {AST::TreeType::IdentifierWithTypeDescriptor, {Id, Id2}};
            return {};
        }
    } // Hoshi
} // Parser