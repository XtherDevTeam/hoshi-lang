//
// Created by theflysong on 2022/8/28.
//

#ifndef XSCRIPT2_UNIQUEEXPRESSIONCODEGEN_HPP
#define XSCRIPT2_UNIQUEEXPRESSIONCODEGEN_HPP

#include <Codegen/Codegen.hpp>
#include <Parsers/UniqueExpressionNode.hpp>

namespace Hoshi {
    class UniqueExpressionCodegen : public Codegen<Operand, UniqueExpressionNode> {
        /**
         * @brief Construct a unique expression codegen
         */
        UniqueExpressionCodegen(void);
    public:
        /**
         * @brief the instance of unique expression Codegen
         */
        static UniqueExpressionCodegen INSTANCE;
        /**
         * @brief visit an unique expression ast and gen the code
         * @return the result of unique expression
         */
        virtual Operand Visit(UniqueExpressionNode &Node, IRProgram::Builder &Program) override;
    };
}

#endif