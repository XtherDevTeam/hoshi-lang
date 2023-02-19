#include <iostream>
#include <pass/pass.hpp>

int main(int argc, const char **argv) {
    auto fp = fopen(argv[1], "r+");
    if (!fp)
        throw std::runtime_error("invalid filename");
    fseek(fp, 0, SEEK_END);
    auto size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto *a = new std::string(size, 0);
    fread(a->data(), size, 1, fp);
    auto *b = new hoshi::wstr{hoshi::string2wstring(*a)};
    delete a;

    auto &&lex = hoshi::pass<hoshi::wstr, hoshi::lexer>({*b});
    hoshi::hoshiModule *mod = hoshi::pass<hoshi::lexer, hoshi::hoshiModule *>(std::move(lex)).get();
    delete b;
    return 0;
}