//
// Created by theflysong on 2022/8/27.
//

#include <Codegen/PrimaryCodegen.hpp>
#include <Codegen/LiteralsCodegen.hpp>

namespace Hoshi {
    /**
     * @brief the instance of literals Codegen
     */
    PrimaryCodegen PrimaryCodegen::INSTANCE;

    /**
     * @brief visit an literal ast and gen the code
     * @return string form of the value of literal
     */
    Operand PrimaryCodegen::Visit(AST &ast, IRProgram::Builder &Program, IRBlock::Builder &Block) {
        if (ast.Type == AST::TreeType::Literals) {
            return LiteralsCodegen::INSTANCE.Visit(ast, Program, Block);
        }
        return Operand::Empty;
    }
}