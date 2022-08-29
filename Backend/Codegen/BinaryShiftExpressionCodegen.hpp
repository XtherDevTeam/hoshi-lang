//
// Created by chou on 2022/8/29.
//

#ifndef XSCRIPT2_BINARYSHIFTEXPRESSIONCODEGEN_HPP
#define XSCRIPT2_BINARYSHIFTEXPRESSIONCODEGEN_HPP

#include "Codegen.hpp"
#include "Parsers/BinaryShiftExpressionNode.hpp"

namespace Hoshi {
    class BinaryShiftExpressionCodegen : public Codegen<Operand, BinaryShiftExpressionNode> {
        /**
         * @brief Construct a BinaryShift expression codegen
         */
        BinaryShiftExpressionCodegen(void);
    public:
        /**
         * @brief the instance of BinaryShift expression Codegen
         */
        static BinaryShiftExpressionCodegen INSTANCE;
        /**
         * @brief visit an BinaryShift expression ast and gen the code
         * @return the result of BinaryShift expression
         */
        virtual Operand Visit(BinaryShiftExpressionNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) override;
    };
}

#endif