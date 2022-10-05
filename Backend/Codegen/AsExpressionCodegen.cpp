//
// Created by p0010 on 22-9-23.
//

#include "AsExpressionCodegen.hpp"

#include "IR/NameUtil.hpp"

#include "BooleanExpressionCodegen.hpp"
#include "IR/BinaryExprIR.hpp"

namespace Hoshi {
    /**
     * @brief Construct a As expression codegen
     */
    AsExpressionCodegen::AsExpressionCodegen(void) = default;

    /**
     * @brief the instance of As expression Codegen
     */
    AsExpressionCodegen AsExpressionCodegen::INSTANCE;

    /**
     * @brief visit an As expression ast and gen the code
     * @return the result of As expression
     */
    Operand AsExpressionCodegen::Visit(AsExpressionNode &Node, IRProgram::Builder &Program) {
        IRBlock::Builder &Block = *Program.GetContext().CurrentBlock;
        Operand LastResult = BooleanExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(0), Program); //The result of last expression ir
        for (int i = 0; i < Node.GetOperators().size(); i++) {
            Lexer::Token OperatorNode = Node.GetOperators(i);
            Operand ThisOperand = BooleanExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(i + 1), Program); //the operand of this ir
            Operand Result = Operand(OperandType::Identifier,
                                     LocalNamePrefix + NewVarName(L"expr")); // the result of this ir
            Opcode Operator;
            Operator = Opcode::As;
            Block.AddIR(BinaryExprIR(Operator, LastResult, ThisOperand, Result));
            LastResult = Result;
        }
        return LastResult;
    }
} // Hoshi