//
// Created by theflysong on 2022/8/27.
//

#ifndef XSCRIPT2_LITERALSCODEGEN_HPP
#define XSCRIPT2_LITERALSCODEGEN_HPP

#include <Config.hpp>
#include <Codegen/Codegen.hpp>

namespace Hoshi {
    class LiteralsCodegen : public Codegen<Operand> {
        /**
         * @brief Construct a literals codegen
         */
        LiteralsCodegen(void) = default;
    public:
        /**
         * @brief the instance of literals Codegen
         */
        static LiteralsCodegen INSTANCE;
        /**
         * @brief visit an literal ast and gen the code
         * @return string form of the value of literal
         */
        virtual Operand Visit(AST &ast, IRProgram::Builder &program, IRBlock::Builder &block) override final;
    };
}

#endif