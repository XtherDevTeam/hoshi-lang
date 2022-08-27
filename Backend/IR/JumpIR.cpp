//
// Created by theflysong on 2022/8/27.
//

#include <IR/JumpIR.hpp>

namespace Hoshi {
    /**
     * @brief Construct a Jump IR
     * @param Destination Jump Destination
     * @param Arguments Arguments of Jump Destination
     */
    JumpIR::JumpIR(Operand Destination, std::initializer_list<Operand> &&Arguments)
        : IR(Opcode::JUMP, Destination, Operand(Arguments), Operand::Empty) {
    }
    /**
     * @brief Construct a Jump IR
     * @param Destination Jump Destination
     * @param Arguments Arguments of Jump Destination
     */
    JumpIR::JumpIR(Operand Destination, std::vector<Operand> &&Arguments)
        : IR(Opcode::JUMP, Destination, Operand(std::move(Arguments)), Operand::Empty) {
    }
    /**
     * @brief Construct a Branch IR
     * @param Condition Branch Condition
     * @param If Jump If Condition is True
     * @param Else Jump If Condition is False
     */
    BranchIR::BranchIR(Operand Condition, Operand If, Operand Else)
        : IR(Opcode::BRANCH, Condition, Operand(std::vector<Operand>({If, Else})), Operand::Empty) {
    }
}