#include <iostream>
#include <fstream>
#include <chrono>
#include "Frontend/Lexer.hpp"
#include "Frontend/AST.hpp"
#include "Frontend/Parsers/AsExpressionParser.hpp"

int main() {
    std::wstring S;
    std::fstream is("../Tests/test.hoshi", std::ios::in);
    while (!is.eof()) {
        std::string f;
        std::getline(is, f);
        S.append(Hoshi::string2wstring(f));
    }
    Hoshi::Lexer Lex(S);
    Lex.Scan();
    Hoshi::AST Tree = Hoshi::Parser::AsExpressionParser(Lex).Parse();
    return 0;
}
