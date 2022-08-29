//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_BINARYMOVEEXPRESSIONCODEGEN_HPP
#define XSCRIPT2_BINARYMOVEEXPRESSIONCODEGEN_HPP

#include "Codegen.hpp"
#include "Parsers/BinaryMoveExpressionNode.hpp"

namespace Hoshi {
    class BinaryMoveExpressionCodegen : public Codegen<Operand, BinaryMoveExpressionNode> {
        /**
         * @brief Construct a BinaryMove expression codegen
         */
        BinaryMoveExpressionCodegen(void);
    public:
        /**
         * @brief the instance of BinaryMove expression Codegen
         */
        static BinaryMoveExpressionCodegen INSTANCE;
        /**
         * @brief visit an BinaryMove expression ast and gen the code
         * @return the result of BinaryMove expression
         */
        virtual Operand Visit(BinaryMoveExpressionNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) override;
    };
}

#endif