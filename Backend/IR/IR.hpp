//
// Created by theflysong on 2022/8/26.
//

#ifndef XSCRIPT2_IR_HPP
#define XSCRIPT2_IR_HPP

#include <IR/Operand.hpp>

namespace Hoshi {
    enum class Opcode : int {
        Nop,
        Add, // +
        Subtract, // -
        Multiply, // *
        Divide, // /
        Remainder, // %
        ShiftRight, // >>
        ShiftLeft, // <<
        And, // &
        Or,  // |
        Xor, // ^
        Equal, // ==
        NotEqual, // !=
        Great, // >
        Less, // <
        GreatEqual, // >=
        LessEqual,  // <=
        Not, // !
        Negative, // -
        Invert,  // ~
        As, // as
        Jump,
        Branch
    };

    /**
     * @brief Code to String
     * @return String form of the code
     */
    const XString ToString(Opcode Code);

    class IR {
    protected:
        /**
         * @brief Code of the IR
         */
        const Opcode Code;
        /**
         * @brief First Source Operand
         */
        const Operand SourceOperandA;
        /**
         * @brief Second Source Operand
         */
        const Operand SourceOperandB;
        /**
         * @brief Destination Operand
         */
        const Operand DestinationOperand;
    public:
        /**
         * @brief Construct an IR
         * @param SourceOperandA First Source Operand
         * @param SourceOperandB Second Source Operand
         * @param DestinationOperand Destination Operand
         */
        IR(const Opcode Code, const Operand SourceOperandA, const Operand SourceOperandB, const Operand DestinationOperand);
        /**
         * @brief Get the code of the IR
         * @return Code of the IR
         */
        const Opcode GetCode(void) const;
        /**
         * @brief Get the first source Operand of the IR
         * @return First Source Operand of the IR
         */
        const Operand GetSourceOperandA(void) const;
        /**
         * @brief Get the second source Operand of the IR
         * @return Second Source Operand of the IR
         */
        const Operand GetSourceOperandB(void) const;
        /**
         * @brief Get the destination Operand of the IR
         * @return Destination Operand of the IR
         */
        const Operand GetDestinationOperand(void) const;
        friend const XString ToString(IR Code);
    };

    /**
     * @brief IR to String
     * @return String form of the ir
     */
    const XString ToString(IR ir);
}

#endif