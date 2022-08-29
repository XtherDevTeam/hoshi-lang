//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_EXPRESSIONSTMTCODEGEN_HPP
#define XSCRIPT2_EXPRESSIONSTMTCODEGEN_HPP

#include <Codegen/Codegen.hpp>
#include <Parsers/ExpressionStmtNode.hpp>

namespace Hoshi {
    class ExpressionStmtCodegen : public Codegen<Operand, ExpressionStmtNode> {
        /**
         * @brief Construct a expression codegen
         */
        ExpressionStmtCodegen(void);
    public:
        /**
         * @brief the instance of expression Codegen
         */
        static ExpressionStmtCodegen INSTANCE;
        /**
         * @brief visit an expression ast and gen the code
         * @return the value of expression
         */
        virtual Operand Visit(ExpressionStmtNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) override;
    };
}

#endif