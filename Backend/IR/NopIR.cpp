//
// Created by theflysong on 2022/8/26.
//

#include <IR/NopIR.hpp>

namespace Hoshi {
    /**
     * @brief Construct a Nop IR
     */
    NopIR::NopIR(void)
        : IR(Opcode::Nop, Operand::Empty, Operand::Empty, Operand::Empty) {
    }
}