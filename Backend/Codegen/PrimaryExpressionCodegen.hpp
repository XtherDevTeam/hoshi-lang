//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_PRIMARYEXPRESSIONCODEGEN_HPP
#define XSCRIPT2_PRIMARYEXPRESSIONCODEGEN_HPP

#include <Codegen/Codegen.hpp>
#include <Parsers/PrimaryExpressionNode.hpp>

namespace Hoshi {
    class PrimaryExpressionCodegen : public Codegen<Operand, PrimaryExpressionNode> {
        /**
         * @brief Construct a primary codegen
         */
        PrimaryExpressionCodegen(void);
    public:
        /**
         * @brief the instance of primary Codegen
         */
        static PrimaryExpressionCodegen INSTANCE;
        /**
         * @brief visit an primary expression ast and gen the code
         * @return the result of primary expression
         */
        virtual Operand Visit(PrimaryExpressionNode &Node, IRProgram::Builder &Program) override;
    };
}

#endif