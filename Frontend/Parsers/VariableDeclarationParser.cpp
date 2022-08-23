//
// Created by p0010 on 22-8-23.
//

#include "VariableDeclarationParser.hpp"
#include "IdentifierWithTypeDescriptorParser.hpp"
#include "AsExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        VariableDeclarationParser::VariableDeclarationParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST VariableDeclarationParser::Parse() {
            if (L.LastToken != (Lexer::Token) {Lexer::TokenKind::Keywords, L"var", 0, 0}) {
                return {};
            }
            L.Scan();
            XArray<AST> ASTs;
            AST Temp;
            do {
                Temp = IdentifierWithTypeDescriptorParser(L).Parse();
                if (Temp.IsNotMatchNode())
                    break;
                if (L.LastToken.Kind == Lexer::TokenKind::AssignSign) {
                    AST Operator = {AST::TreeType::Operator, L.LastToken};
                    L.Scan();
                    AST RHS = AsExpressionParser(L).Parse();
                    Temp = {AST::TreeType::AssignmentExpression, {Temp, Operator, RHS}};
                }
                ASTs.push_back(Temp);
            } while (!Temp.IsNotMatchNode());
            return {AST::TreeType::VariableDeclaration, ASTs};
        }
    } // Hoshi
} // Parser