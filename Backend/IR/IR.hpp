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
    XString ToString(Opcode Code);

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
        IR(Opcode Code, Operand SourceOperandA, Operand SourceOperandB,
           Operand DestinationOperand);

        /**
         * @brief Get the code of the IR
         * @return Code of the IR
         */
        Opcode GetCode() const;

        /**
         * @brief Get the first source Operand of the IR
         * @return First Source Operand of the IR
         */
        Operand GetSourceOperandA() const;

        /**
         * @brief Get the second source Operand of the IR
         * @return Second Source Operand of the IR
         */
        Operand GetSourceOperandB() const;

        /**
         * @brief Get the destination Operand of the IR
         * @return Destination Operand of the IR
         */
        Operand GetDestinationOperand() const;

        friend XString ToString(IR Code);
    };

    /**
     * @brief IR to String
     * @return String form of the ir
     */
    XString ToString(IR ir);
}

#endif