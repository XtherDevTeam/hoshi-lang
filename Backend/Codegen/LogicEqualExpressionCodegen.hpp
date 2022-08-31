//
// Created by chou on 8/31/22.
//

#ifndef HOSHI_LANG_LOGICEQUALEXPRESSIONCODEGEN_HPP
#define HOSHI_LANG_LOGICEQUALEXPRESSIONCODEGEN_HPP

#include "IR/Operand.hpp"
#include "Codegen.hpp"
#include "Parsers/LogicEqualExpressionNode.hpp"

namespace Hoshi {

    class LogicEqualExpressionCodegen : public Codegen<Operand, LogicEqualExpressionNode> {
/**
         * @brief Construct a LogicEqual expression codegen
         */
        LogicEqualExpressionCodegen();

    public:
        /**
         * @brief the instance of LogicEqual expression Codegen
         */
        static LogicEqualExpressionCodegen INSTANCE;

        /**
         * @brief visit an LogicEqual expression ast and gen the code
         * @return the result of BinaryShift expression
         */
        Operand
        Visit(LogicEqualExpressionNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) override;
    };

} // Hoshi

#endif //HOSHI_LANG_LOGICEQUALEXPRESSIONCODEGEN_HPP
