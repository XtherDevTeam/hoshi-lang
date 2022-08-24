#include <iostream>
#include <fstream>
#include "Frontend/Lexer.hpp"
#include "Frontend/AST.hpp"
#include "Frontend/Parsers/FunctionDefinitionParser.hpp"

int main() {
    std::wstring S;
    std::fstream is("../Tests/test.hoshi", std::ios::in);
    while (!is.eof()) {
        std::string f;
        std::getline(is, f);
        S.append(Hoshi::string2wstring(f + "\n"));
    }
    Hoshi::Lexer Lex(S);
    Lex.Scan();
    Hoshi::AST Tree = Hoshi::Parser::FunctionDefinitionParser(Lex).Parse();
    return 0;
}
