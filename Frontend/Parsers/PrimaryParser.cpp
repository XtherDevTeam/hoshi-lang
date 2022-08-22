//
// Created by Jerry Chou on 22-8-22.
//

#include "PrimaryParser.hpp"
#include "StaticMemberAccessExpressionParser.hpp"
#include "LiteralsParser.hpp"
#include "FunctionInvokeExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        PrimaryParser::PrimaryParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST PrimaryParser::Parse() {
            AST Tree = FunctionInvokeExpressionParser(L).Parse();
            if (!Tree.IsNotMatchNode())
                return Tree;
            Tree = LiteralsParser(L).Parse();
            if (!Tree.IsNotMatchNode())
                return Tree;

            return {};
        }
    } // Hoshi
} // Parser