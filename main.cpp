#include <iostream>
#include <fstream>
#include "Frontend/Lexer.hpp"
#include "Frontend/AST.hpp"
#include "Frontend/Parsers/MultiplicationExpressionParser.hpp"
#include <Passes/ToASTPass.hpp>
#include <Passes/ToIRPass.hpp>
#include <Passes/XStringInputPassResult.hpp>
#include <Exceptions/CompilerError.hpp>

class DefaultASTPassResult : public Hoshi::ASTPassResult {
public:
    friend int main(int argc, const char **argv);
};

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
        try {
            Hoshi::XStringInputPassResult FirstPassResult(S);
            Hoshi::ToASTPass ASTPass(FirstPassResult);
            Hoshi::ToIRPass IRPass(ASTPass.GetResult());
            ASTPass.Run();
            IRPass.Run();
            Hoshi::IRProgram Program = IRPass.GetResult().GetResult().build();
            Hoshi::IRBlock Block = Program.GetBlocks()[0];
            for (Hoshi::IR ir : Block.GetIRCollection()) {
                std::wcout << Hoshi::ToString(ir) << std::endl;
            }
        } catch(Hoshi::CompilerError e) {
            std::wcerr << e.what() << std::endl;
        } catch(Hoshi::HoshiException e) {
            std::wcerr << e.what() << std::endl;
        }
    } catch (Hoshi::HoshiException e) {
        std::wcout << e.what() << std::endl;
    } catch (Hoshi::LexerException e) {
        std::wcout << e.what() << std::endl;
    }
    return 0;
}
