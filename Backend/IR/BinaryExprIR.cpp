//
// Created by theflysong on 2022/8/27.
//

#include <IR/BinaryExprIR.hpp>

namespace Hoshi {
    /**
     * @brief Construct a Binary Expression IR
     * @param Code Type of Binary Expression
     * @param SourceOperendA First Source Oprend
     * @param SourceOperendB Second Source Oprend
     * @param DestinationOperand Destination Oprend
     */
    BinaryExprIR::BinaryExprIR(Opcode Code, Operand SourceOperandA, Operand SourceOperandB, Operand DestinationOperand)
            : IR(Code, SourceOperandA, SourceOperandB, DestinationOperand) {
    }
}