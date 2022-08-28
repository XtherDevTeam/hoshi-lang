//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_UNIQUEEXPRIR_HPP
#define XSCRIPT2_UNIQUEEXPRIR_HPP

#include <IR/IR.hpp>

namespace Hoshi {
    class UniqieExprIR : public IR {
    public:
        /**
         * @brief Construct a Uniqie Expression IR
         * @param Code Type of Uniqie Expression
         * @param SourceOperend Source Oprend
         * @param DestinationOperand Destination Oprend
         */
        UniqieExprIR(Opcode Code, Operand SourceOperand, Operand DestinationOperand);
    };
}

#endif