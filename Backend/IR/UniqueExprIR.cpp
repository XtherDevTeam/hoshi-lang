//
// Created by theflysong on 2022/8/27.
//

#include <IR/UniqueExprIR.hpp>

namespace Hoshi {
    /**
    * @brief Construct a Uniqie Expression IR
    * @param Code Type of Uniqie Expression
    * @param SourceOperend Source Oprend
    * @param DestinationOperand Destination Oprend
    */
    UniqieExprIR::UniqieExprIR(Opcode Code, Operand SourceOperand, Operand DestinationOperand)
        : IR(Code, SourceOperand, Operand::Empty, DestinationOperand) {
    }
}