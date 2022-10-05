//
// Created by p0010 on 22-9-23.
//

#ifndef HOSHI_LANG_BOOLEANEXPRESSIONCODEGEN_HPP
#define HOSHI_LANG_BOOLEANEXPRESSIONCODEGEN_HPP

#include "IR/Operand.hpp"
#include "Parsers/BooleanExpressionNode.hpp"
#include "Codegen.hpp"

namespace Hoshi {
    class BooleanExpressionCodegen : public Codegen<Operand, BooleanExpressionNode> {
        /**
         * @brief Construct a Boolean expression codegen
         */
        BooleanExpressionCodegen();

    public:
        /**
         * @brief the instance of Boolean expression Codegen
         */
        static BooleanExpressionCodegen INSTANCE;

        /**
         * @brief visit an Boolean expression ast and gen the code
         * @return the result of BooleanShift expression
         */
        Operand
        Visit(BooleanExpressionNode &Node, IRProgram::Builder &Program) override;
    };

} // Hoshi

#endif //HOSHI_LANG_BOOLEANEXPRESSIONCODEGEN_HPP
