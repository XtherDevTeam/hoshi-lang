//
// Created by p0010 on 22-8-24.
//

#include "SimpleStatementParser.hpp"
#include "AsExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        SimpleStatementParser::SimpleStatementParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST SimpleStatementParser::Parse() {
            if (L.LastToken.Kind == Lexer::TokenKind::Keywords) {
                /**
                 * @biref return control statement
                 */
                if (L.LastToken.Value == L"return") {
                    L.Scan();
                    AST Result = AsExpressionParser(L).Parse();
                    if (Result.IsNotMatchNode())
                        return {AST::TreeType::ReturnStatement, (XArray<AST>){}};
                    return {AST::TreeType::ReturnStatement, {Result}};
                }
                    /**
                     * @biref continue control statement
                     */
                else if (L.LastToken.Value == L"continue") {
                    L.Scan();
                    return {AST::TreeType::ContinueStatement, (Lexer::Token) {}};
                }
                    /**
                     * @biref break control statement
                     */
                else if (L.LastToken.Value == L"break") {
                    L.Scan();
                    return {AST::TreeType::BreakStatement, (Lexer::Token) {}};
                }
                else {
                    return {};
                }
            } else {
                return {};
            }
        }
    } // Hoshi
} // Parser