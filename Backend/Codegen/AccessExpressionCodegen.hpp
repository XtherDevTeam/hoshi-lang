//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_ACCESSEXPRESSIONCODEGEN_HPP
#define XSCRIPT2_ACCESSEXPRESSIONCODEGEN_HPP

#include <Codegen/Codegen.hpp>
#include <Parsers/AccessExpressionNode.hpp>

namespace Hoshi {
    class AccessExpressionCodegen : public Codegen<Operand, AccessExpressionNode> {
        /**
         * @brief Construct an access expression codegen
         */
        AccessExpressionCodegen(void);
    public:
        /**
         * @brief the instance of access expression Codegen
         */
        static AccessExpressionCodegen INSTANCE;
        /**
         * @brief visit an access expression ast and gen the code
         * @return the value of access expression
         */
        virtual Operand Visit(AccessExpressionNode &Node, IRProgram::Builder &Program) override;
        virtual VariableSymbolInfo &VisitSymbol(AccessExpressionNode &Node, IRProgram::Builder &Program);
    };
}

#endif