#include <iostream>
#include <fstream>
#include "Frontend/Lexer.hpp"
#include "Frontend/AST.hpp"
#include "Frontend/Parsers/InterfaceDefinitionParser.hpp"

int main(int argc, const char **argv) {
    std::wstring S;
    if (argc != 2) {
        return -1;
    }
    std::fstream is(argv[1], std::ios::in);
    while (!is.eof()) {
        std::string f;
        std::getline(is, f);
        S.append(Hoshi::string2wstring(f + "\n"));
    }
    Hoshi::Lexer Lex(S);
    Lex.Scan();
    Hoshi::AST Tree = Hoshi::Parser::InterfaceDefinitionParser(Lex).Parse();
    return 0;
}
