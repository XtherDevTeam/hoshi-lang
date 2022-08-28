//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_ADDITIONEXPRESSIONCODEGEN_HPP
#define XSCRIPT2_ADDITIONEXPRESSIONCODEGEN_HPP

#include <Config.hpp>
#include <Codegen/Codegen.hpp>

namespace Hoshi {
    class AdditionExpressionCodegen : public Codegen<Operand> {
        /**
         * @brief Construct a addition expression codegen
         */
        AdditionExpressionCodegen(void) = default;
    public:
        /**
         * @brief the instance of addition expression Codegen
         */
        static AdditionExpressionCodegen INSTANCE;
        /**
         * @brief visit an addition expression ast and gen the code
         * @return result variable of addition expression
         */
        virtual Operand Visit(AST &ast, IRProgram::Builder &Program, IRBlock::Builder &Block) override final;
    };
}

#endif