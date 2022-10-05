//
// Created by chou on 8/30/22.
//

#ifndef HOSHI_LANG_LOGICCOMPARINGEXPRESSIONCODEGEN_HPP
#define HOSHI_LANG_LOGICCOMPARINGEXPRESSIONCODEGEN_HPP

#include "Codegen.hpp"
#include "Parsers/LogicComparingExpressionNode.hpp"

namespace Hoshi {

    class LogicComparingExpressionCodegen : public Codegen<Operand, LogicComparingExpressionNode> {
        /**
         * @brief Construct a LogicComparing expression codegen
         */
        LogicComparingExpressionCodegen();

    public:
        /**
         * @brief the instance of LogicComparing expression Codegen
         */
        static LogicComparingExpressionCodegen INSTANCE;

        /**
         * @brief visit an LogicComparing expression ast and gen the code
         * @return the result of BinaryShift expression
         */
        Operand
        Visit(LogicComparingExpressionNode &Node, IRProgram::Builder &Program) override;
    };

} // Hoshi

#endif //HOSHI_LANG_LOGICCOMPARINGEXPRESSIONCODEGEN_HPP
