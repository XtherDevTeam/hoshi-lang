//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_MULIPLICATIONEXPRESSIONCODEGEN_HPP
#define XSCRIPT2_MULIPLICATIONEXPRESSIONCODEGEN_HPP

#include <Codegen/Codegen.hpp>
#include <Parsers/MuliplicationExpressionNode.hpp>

namespace Hoshi {
    class MuliplicationExpressionCodegen : public Codegen<Operand, MuliplicationExpressionNode> {
        /**
         * @brief Construct a muliplication expression codegen
         */
        MuliplicationExpressionCodegen(void);
    public:
        /**
         * @brief the instance of muliplication expression Codegen
         */
        static MuliplicationExpressionCodegen INSTANCE;
        /**
         * @brief visit an muliplication expression ast and gen the code
         * @return the result of muliplication expression
         */
        virtual Operand Visit(MuliplicationExpressionNode &Node, IRProgram::Builder &Program) override;
    };
}

#endif