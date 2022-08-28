//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_BINARYEXPRIR_HPP
#define XSCRIPT2_BINARYEXPRIR_HPP

#include <IR/IR.hpp>

namespace Hoshi {
    class BinaryExprIR : public IR {
    public:
        /**
         * @brief Construct a Binary Expression IR
         * @param Code Type of Binary Expression
         * @param SourceOperendA First Source Oprend
         * @param SourceOperendB Second Source Oprend
         * @param DestinationOperand Destination Oprend
         */
        BinaryExprIR(Opcode Code, Operand SourceOperandA, Operand SourceOperandB, Operand DestinationOperand);
    };
}

#endif