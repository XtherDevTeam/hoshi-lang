//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_MULTIPLICATIONEXPRESSIONCODEGEN_HPP
#define XSCRIPT2_MULTIPLICATIONEXPRESSIONCODEGEN_HPP

#include <Config.hpp>
#include <Codegen/Codegen.hpp>

namespace Hoshi {
    class MultiplicationExpressionCodegen : public Codegen<Operand> {
        /**
         * @brief Construct a multiplication expression codegen
         */
        MultiplicationExpressionCodegen(void) = default;
    public:
        /**
         * @brief the instance of multiplication expression Codegen
         */
        static MultiplicationExpressionCodegen INSTANCE;
        /**
         * @brief visit an multiplication expression ast and gen the code
         * @return result variable of multiplication expression
         */
        virtual Operand Visit(AST &ast, IRProgram::Builder &Program, IRBlock::Builder &Block) override final;
    };
}

#endif