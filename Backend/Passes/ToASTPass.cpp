//
// Created by chou on 8/28/22.
//

#include "ToASTPass.hpp"
#include "Parsers/GlobalStmtParser.hpp"
#include "Exceptions/ParserException.hpp"

namespace Hoshi {
    ToASTPass::ToASTPass(XStringPassResult LastPass) : Pass(LastPass, PassResult) {

    }

    void ToASTPass::Run() {
        Lexer Lex(LastPass.GetResult());
        XArray<AST> Trees;
        AST Temp = Hoshi::Parser::GlobalStmtParser(Lex).Parse();
        while (Temp.IsNotMatchNode()) {
            if(Lex.LastToken.Kind == Lexer::TokenKind::Semicolon) {
                Lex.Scan();
            } else {
                if (Temp.Type == AST::TreeType::FunctionDefinition) {
                    // do nothing
                } else {
                    throw ParserException(Lex.Line, Lex.Column, L"Expected a `;`");
                }
            }
            Trees.push_back(Temp);
        }
        PassResult.Result = {Hoshi::AST::TreeType::File, std::move(Trees)};
    }
} // Hoshi