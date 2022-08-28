#include <iostream>
#include <fstream>
#include "Frontend/Lexer.hpp"
#include "Frontend/AST.hpp"
#include "Frontend/Parsers/SingleExpressionParser.hpp"
#include <Codegen/SingleExpressionCodegen.hpp>

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
    try {
        Hoshi::Lexer Lex(S);
        Lex.Scan();
        Hoshi::AST Tree = Hoshi::Parser::SingleExpressionParser(Lex).Parse();

        
    } catch (Hoshi::HoshiException e) {
        std::wcout << e.what() << std::endl;
    } catch (Hoshi::LexerException e) {
        std::wcout << e.what() << std::endl;
    }
    return 0;
}
