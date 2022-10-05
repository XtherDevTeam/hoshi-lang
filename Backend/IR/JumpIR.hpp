//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_JUMPIR_HPP
#define XSCRIPT2_JUMPIR_HPP

#include <IR/IR.hpp>

namespace Hoshi {
    class JumpIR : public IR {
    public:
        /**
         * @brief Construct a Jump IR
         * @param Destination Jump Destination
         * @param Arguments Arguments of Jump Destination
         */
        JumpIR(Operand Destination, std::initializer_list<Operand> &&Arguments);

        /**
         * @brief Construct a Jump IR
         * @param Destination Jump Destination
         * @param Arguments Arguments of Jump Destination
         */
        JumpIR(Operand Destination, XArray<Operand> &&Arguments);
    };

    class BranchIR : public IR {
    public:
        /**
         * @brief Construct a Branch IR
         * @param Condition Branch Condition
         * @param If Jump If Condition is True
         * @param Else Jump If Condition is False
         */
        BranchIR(Operand Condition, Operand If, Operand Else);
    };
}

#endif