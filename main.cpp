#include <iostream>
#include <fstream>
#include <chrono>
#include "Frontend/Lexer.hpp"

int main() {
    std::wstring S;
    std::fstream is("../Tests/test.hoshi", std::ios::in);
    while (!is.eof()) {
        std::string f;
        std::getline(is, f);
        S.append(Hoshi::string2wstring(f));
    }
    Hoshi::Lexer Lex(S);
    Hoshi::XIndexType ReadTokenCount = 0;
    while (Lex.Scan().Kind != Hoshi::Lexer::TokenKind::EoF) {
        ReadTokenCount++;
    }
    std::cout << "Summary: Token count : " << ReadTokenCount << " File size " << S.length() << std::endl;
    return 0;
}
