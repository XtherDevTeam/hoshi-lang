//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_SINGLEEXPRESSIONCODEGEN_HPP
#define XSCRIPT2_SINGLEEXPRESSIONCODEGEN_HPP

#include <Config.hpp>
#include <Codegen/Codegen.hpp>

namespace Hoshi {
    class SingleExpressionCodegen : public Codegen<Operand> {
        /**
         * @brief Construct a single expression codegen
         */
        SingleExpressionCodegen(void) = default;
    public:
        /**
         * @brief the instance of single expression Codegen
         */
        static SingleExpressionCodegen INSTANCE;
        /**
         * @brief visit an single expression ast and gen the code
         * @return result variable of Single expression and the ir of single expression
         */
        virtual Operand Visit(AST &ast, IRProgram::Builder &program, IRBlock::Builder &block) override final;
    };
}

#endif