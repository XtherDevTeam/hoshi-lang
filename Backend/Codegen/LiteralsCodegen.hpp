//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_LITERALSCODEGEN_HPP
#define XSCRIPT2_LITERALSCODEGEN_HPP

#include <Codegen/Codegen.hpp>
#include <Parsers/LiteralsNode.hpp>

namespace Hoshi {
    class LiteralsCodegen : public Codegen<Operand, LiteralsNode> {
        /**
         * @brief Construct a literals codegen
         */
        LiteralsCodegen(void);
    public:
        /**
         * @brief the instance of literals Codegen
         */
        static LiteralsCodegen INSTANCE;
        /**
         * @brief visit a literal ast and gen the code
         * @return string form of the value of literal
         */
        virtual Operand Visit(LiteralsNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) override;
    };
}

#endif