//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_ADDITIONEXPRESSIONCODEGEN_HPP
#define XSCRIPT2_ADDITIONEXPRESSIONCODEGEN_HPP

#include <Codegen/Codegen.hpp>
#include <Parsers/AdditionExpressionNode.hpp>

namespace Hoshi {
    class AdditionExpressionCodegen : public Codegen<Operand, AdditionExpressionNode> {
        /**
         * @brief Construct a addition expression codegen
         */
        AdditionExpressionCodegen(void);
    public:
        /**
         * @brief the instance of addition expression Codegen
         */
        static AdditionExpressionCodegen INSTANCE;
        /**
         * @brief visit an addition expression ast and gen the code
         * @return the result of addition expression
         */
        virtual Operand Visit(AdditionExpressionNode &Node, IRProgram::Builder &Program) override;
    };
}

#endif