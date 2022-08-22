//
// Created by Jerry Chou on 22-8-22.
//

#include "FunctionInvokeExpressionParser.hpp"
#include "IdentifierParser.hpp"
#include "ArgumentsParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        FunctionInvokeExpressionParser::FunctionInvokeExpressionParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST FunctionInvokeExpressionParser::Parse() {
            AST LHS = StaticMemberAccessExpressionParser(L).Parse();
            if (LHS.IsNotMatchNode())
                return {};
            AST Arguments = ArgumentsParser(L).Parse();
            if (Arguments.IsNotMatchNode())
                return LHS;
            while (!Arguments.IsNotMatchNode()) {
                LHS = {AST::TreeType::FunctionInvokeExpression, {LHS, Arguments}};
                Arguments = ArgumentsParser(L).Parse();
            }
            return LHS;
        }
    } // Hoshi
} // Parser