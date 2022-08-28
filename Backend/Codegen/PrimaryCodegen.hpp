//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_PRIMARYCODEGEN_HPP
#define XSCRIPT2_PRIMARYCODEGEN_HPP

#include <Config.hpp>
#include <Codegen/Codegen.hpp>

namespace Hoshi {
    class PrimaryCodegen : public Codegen<Operand> {
        /**
         * @brief Construct a primary codegen
         */
        PrimaryCodegen(void) = default;
    public:
        /**
         * @brief the instance of primary Codegen
         */
        static PrimaryCodegen INSTANCE;
        /**
         * @brief visit an primary ast and gen the code
         * @return value of primary
         */
        virtual Operand Visit(AST &ast, IRProgram::Builder &Program, IRBlock::Builder &Block) override final;
    };
}

#endif