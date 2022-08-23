//
// Created by p0010 on 22-8-23.
//

#include "StatementParser.hpp"
#include "CodeBlockParser.hpp"
#include "AsExpressionParser.hpp"
#include "AssignmentExpressionParser.hpp"

namespace Hoshi {
    namespace Parser {
        StatementParser::StatementParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST StatementParser::Parse() {
            AST Result;

            Result = CodeBlockParser(L).Parse();
            if (!Result.IsNotMatchNode())
                return Result;

            Result = AssignmentExpressionParser(L).Parse();
            if (!Result.IsNotMatchNode())
                return Result;

            Result = AsExpressionParser(L).Parse();
            if (!Result.IsNotMatchNode())
                return Result;

            return {};
        }
    } // Hoshi
} // Parser