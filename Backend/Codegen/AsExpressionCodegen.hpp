//
// Created by p0010 on 22-9-23.
//

#ifndef HOSHI_LANG_ASEXPRESSIONCODEGEN_HPP
#define HOSHI_LANG_ASEXPRESSIONCODEGEN_HPP

#include "IR/Operand.hpp"
#include "Parsers/AsExpressionNode.hpp"
#include "Codegen.hpp"

namespace Hoshi {

    class AsExpressionCodegen : public Codegen<Operand, AsExpressionNode> {
        /**
         * @brief Construct a As expression codegen
         */
        AsExpressionCodegen();

    public:
        /**
         * @brief the instance of As expression Codegen
         */
        static AsExpressionCodegen INSTANCE;

        /**
         * @brief visit an As expression ast and gen the code
         * @return the result of AsShift expression
         */
        Operand
        Visit(AsExpressionNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) override;
    };

} // Hoshi

#endif //HOSHI_LANG_ASEXPRESSIONCODEGEN_HPP
