//
// Created by p0010 on 22-9-11.
//

#ifndef HOSHI_LANG_BINARYEXPRESSIONCODEGEN_HPP
#define HOSHI_LANG_BINARYEXPRESSIONCODEGEN_HPP

#include "IR/Operand.hpp"
#include "Codegen.hpp"
#include "Parsers/BinaryExpressionNode.hpp"

namespace Hoshi {

    class BinaryExpressionCodegen : public Codegen<Operand, BinaryExpressionNode> {
/**
         * @brief Construct a Binary expression codegen
         */
        BinaryExpressionCodegen();

    public:
        /**
         * @brief the instance of Binary expression Codegen
         */
        static BinaryExpressionCodegen INSTANCE;

        /**
         * @brief visit an Binary expression ast and gen the code
         * @return the result of BinaryShift expression
         */
        Operand
        Visit(BinaryExpressionNode &Node, IRProgram::Builder &Program) override;
    };

} // Hoshi

#endif //HOSHI_LANG_BINARYEXPRESSIONCODEGEN_HPP
