//
// Created by theflysong on 2022/8/28.
//

#include <Codegen/PrimaryExpressionCodegen.hpp>
#include <Codegen/LiteralsCodegen.hpp>
#include <Codegen/AccessExpressionCodegen.hpp>

namespace Hoshi {
    /**
     * @brief Construct a primary expression codegen
     */
    PrimaryExpressionCodegen::PrimaryExpressionCodegen(void) = default;
    /**
     * @brief the instance of primary expression Codegen
     */
    PrimaryExpressionCodegen PrimaryExpressionCodegen::INSTANCE;
    /**
     * @brief visit an primary expression ast and gen the code
     * @return the result of primary expression
     */
    Operand PrimaryExpressionCodegen::Visit(PrimaryExpressionNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) {
        if (Node.GetLiterals() != NULL) {
            return LiteralsCodegen::INSTANCE.Visit(*Node.GetLiterals(), Program, Block);
        }
        else if (Node.GetAccess() != NULL) {
            return AccessExpressionCodegen::INSTANCE.Visit(*Node.GetAccess(), Program, Block);
        }
        throw CompilerError(Node.Line, Node.Column, L"Invalid Primary Expression!");
    }
}