//
// Created by p0010 on 22-8-27.
//

#include "GlobalStmtParser.hpp"
#include "ClassDefinitionParser.hpp"
#include "FunctionDefinitionParser.hpp"
#include "InterfaceDefinitionParser.hpp"
#include "VariableDeclarationParser.hpp"
#include "ImportStatementParser.hpp"

namespace Hoshi {
    namespace Parser {
        GlobalStmtParser::GlobalStmtParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST GlobalStmtParser::Parse() {
            AST Result;

            Result = ImportStatementParser(L).Parse();
            if (!Result.IsNotMatchNode())
                return Result;

            Result = InterfaceDefinitionParser(L).Parse();
            if (!Result.IsNotMatchNode())
                return Result;

            Result = ClassDefinitionParser(L).Parse();
            if (!Result.IsNotMatchNode())
                return Result;

            Result = FunctionDefinitionParser(L).Parse();
            if (!Result.IsNotMatchNode())
                return Result;

            Result = VariableDeclarationParser(L).Parse();
            if (!Result.IsNotMatchNode())
                return Result;

            return {};
        }
    } // Hoshi
} // Parser