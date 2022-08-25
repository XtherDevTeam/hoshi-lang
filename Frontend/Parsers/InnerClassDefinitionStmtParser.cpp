//
// Created by p0010 on 22-8-25.
//

#include "InnerClassDefinitionStmtParser.hpp"
#include "VariableDeclarationParser.hpp"
#include "MethodDefinitionParser.hpp"

namespace Hoshi {
    namespace Parser {
        InnerClassDefinitionStmtParser::InnerClassDefinitionStmtParser(Lexer &L) : ParserBase(L) {

        }

        Hoshi::AST InnerClassDefinitionStmtParser::Parse() {
            AST Result;

            Result = MethodDefinitionParser(L).Parse();
            if (!Result.IsNotMatchNode())
                return Result;

            Result = VariableDeclarationParser(L).Parse();
            if (!Result.IsNotMatchNode())
                return Result;

            return {};
        }
    } // Hoshi
} // Parser