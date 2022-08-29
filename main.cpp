#include <iostream>
#include <fstream>
#include <Lexer.hpp>
#include <Parsers/MuliplicationExpressionNode.hpp>
#include <Passes/Pass.hpp>
#include <Codegen/MuliplicationExpressionCodegen.hpp>
#include <Exceptions/CompilerError.hpp>

// class DefaultASTPassResult : public Hoshi::ASTPassResult {
// public:
//     friend int main(int argc, const char **argv);
// };

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
        Hoshi::MuliplicationExpressionNode *CST = Hoshi::MuliplicationExpressionNode::Parser::INSTANCE.Parse(Lex);

        Hoshi::IRProgram::Builder Program;
        Program.SetName(L"A");
        Hoshi::IRBlock::Builder Block;
        Block.SetName(L"A");
        Hoshi::MuliplicationExpressionCodegen::INSTANCE.Visit(*CST, Program, Block);
        Program.AddBlock(Block.build());
        Hoshi::IRProgram ProgramResult = Program.build();

        for (Hoshi::IR ir : ProgramResult.GetBlocks()[0].GetIRCollection()) {
            std::wcout << Hoshi::ToString(ir) << std::endl;
        }

        delete CST;
    } catch(Hoshi::CompilerError e) {
         std::wcerr << e.what() << std::endl;
    } catch(Hoshi::ParserException e) {
         std::wcerr << e.what() << std::endl;
    } catch (Hoshi::HoshiException e) {
        std::wcout << e.what() << std::endl;
    } catch (Hoshi::LexerException e) {
        std::wcout << e.what() << std::endl;
    }
    return 0;
}
