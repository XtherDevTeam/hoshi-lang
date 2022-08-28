//
// Created by theflysong on 2022/8/28.
//

#include <Passes/ToIRPass.hpp>
#include <Codegen/MultiplicationExpressionCodegen.hpp>

namespace Hoshi {
    /**
     * @brief Construct a Pass
     * @param LastPass Result of Last Pass
     */
    ToIRPass::ToIRPass(ASTPassResult &ast) 
        : Pass(ast, PassResult), PassResult() {
    }
    /**
     * @brief Run a Pass
     */
    void ToIRPass::Run(void) {
        IRProgram::Builder Program;
        Program.SetName(L"Program");
        IRBlock::Builder Block;
        Block.SetName(L"Start");
        Hoshi::MultiplicationExpressionCodegen::INSTANCE.Visit(LastPass.GetResult(), Program, Block);
        Program.AddBlock(Block.build());
        PassResult.Result = std::move(Program);
    }
}