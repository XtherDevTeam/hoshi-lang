//
// Created by theflysong on 2022/8/26.
//

#include <IR/IR.hpp>

namespace Hoshi {
    /**
     * @brief Construct an IR
     * @param SourceOperandA First Source Operand
     * @param SourceOperandB Second Source Operand
     * @param DestinationOperand Destination Operand
     */
    IR::IR(const Opcode Code, const Operand SourceOperandA, const Operand SourceOperandB, const Operand DestinationOperand)
        : Code(Code), SourceOperandA(SourceOperandA), SourceOperandB(SourceOperandB), DestinationOperand(DestinationOperand) {
    }
    /**
     * @brief Get the code of the IR
     * @return Code of the IR
     */
    const Opcode IR::GetCode(void) const {
        return Code;
    }
    /**
     * @brief Get the first source Operand of the IR
     * @return First Source Operand of the IR
     */
    const Operand IR::GetSourceOperandA(void) const {
        return SourceOperandA;
    }
    /**
     * @brief Get the second source Operand of the IR
     * @return Second Source Operand of the IR
     */
    const Operand IR::GetSourceOperandB(void) const {
        return SourceOperandB;
    }
    /**
     * @brief Get the destination Operand of the IR
     * @return Destination Operand of the IR
     */
    const Operand IR::GetDestinationOperand(void) const {
        return DestinationOperand;
    }
}