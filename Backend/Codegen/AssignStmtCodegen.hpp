//
// Created by theflysong on 2022/8/29.
//

#ifndef XSCRIPT2_ASSIGNSTMTCODEGEN_HPP
#define XSCRIPT2_ASSIGNSTMTCODEGEN_HPP

#include <Codegen/Codegen.hpp>
#include <Parsers/AssignStmtNode.hpp>

namespace Hoshi {
    class AssignStmtCodegen : public Codegen<Operand, AssignStmtNode> {
        /**
         * @brief Construct a assign stmt codegen
         */
        AssignStmtCodegen();

    public:
        /**
         * @brief the instance of assign stmt Codegen
         */
        static AssignStmtCodegen INSTANCE;

        /**
         * @brief visit an assign stmt and gen the code
         * @return the value of assign stmt
         */
        Operand Visit(AssignStmtNode &Node, IRProgram::Builder &Program) override;
    };
}

#endif