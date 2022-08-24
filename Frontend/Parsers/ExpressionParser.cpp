//
// Created by p0010 on 22-8-24.
//

#include "ExpressionParser.hpp"
#include "AssignmentExpressionParser.hpp"
#include "AsExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        ExpressionParser::ExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST ExpressionParser::Parse() {
            AST Expr = AssignmentExpressionParser(L).Parse();
            if (!Expr.IsNotMatchNode()) {
                return Expr;
            }
            Expr = AsExpressionParser(L).Parse();
            if (!Expr.IsNotMatchNode()) {
                return Expr;
            }
            return {};
        }
    } // Hoshi
} // Parser