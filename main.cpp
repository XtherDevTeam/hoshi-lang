#include <iostream>
#include <fstream>
#include <Lexer.hpp>
#include <Passes/Pass.hpp>
#include <Exceptions/CompilerError.hpp>
#include <Parsers/AssignStmtNode.hpp>
#include <Parsers/ExpressionStmtNode.hpp>
#include <Codegen/AssignStmtCodegen.hpp>
#include <Codegen/ExpressionStmtCodegen.hpp>

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
        Hoshi::AssignStmtNode *CST0 = Hoshi::AssignStmtNode::Parser::INSTANCE.Parse(Lex);
        Hoshi::ExpressionStmtNode *CST1 = Hoshi::ExpressionStmtNode::Parser::INSTANCE.Parse(Lex);

        Hoshi::IRProgram::Builder Program;
        Program.SetName(L"A");
        Hoshi::IRBlock::Builder Block;
        Block.SetName(L"A");
        Hoshi::AssignStmtCodegen::INSTANCE.Visit(*CST0, Program, Block);
        Hoshi::ExpressionStmtCodegen::INSTANCE.Visit(*CST1, Program, Block);
        Program.AddBlock(Block.build());
        Hoshi::IRProgram ProgramResult = Program.build();

        for (Hoshi::IR ir: ProgramResult.GetBlocks()[0].GetIRCollection()) {
            std::wcout << Hoshi::ToString(ir) << std::endl;
        }

        delete CST0;
        delete CST1;
    } catch (const Hoshi::CompilerError &e) {
        std::wcerr << e.what() << std::endl;
    } catch (const Hoshi::ParserException &e) {
        std::wcerr << e.what() << std::endl;
    } catch (const Hoshi::HoshiException &e) {
        std::wcout << e.what() << std::endl;
    } catch (const Hoshi::LexerException &e) {
        std::wcout << e.what() << std::endl;
    }
    return 0;
}
