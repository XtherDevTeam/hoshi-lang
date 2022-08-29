//
// Created by theflysong on 2022/8/29.
//

#include <Codegen/ExpressionStmtCodegen.hpp>
#include <Codegen/ExpressionCodegen.hpp>

namespace Hoshi {
    /**
     * @brief Construct a expression codegen
     */
    ExpressionStmtCodegen::ExpressionStmtCodegen(void) = default;
    /**
     * @brief the instance of expression Codegen
     */
    ExpressionStmtCodegen ExpressionStmtCodegen::INSTANCE;
    /**
     * @brief visit an expression ast and gen the code
     * @return the value of expression
     */
    Operand ExpressionStmtCodegen::Visit(ExpressionStmtNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) {
        return ExpressionCodegen::INSTANCE.Visit(*Node.GetExpression(), Program, Block);
    }
}