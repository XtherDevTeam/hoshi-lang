//
// Created by theflysong on 2022/8/29.
//

#include <Codegen/AccessExpressionCodegen.hpp>

namespace Hoshi {
    AccessExpressionCodegen::AccessExpressionCodegen(void) = default;

    AccessExpressionCodegen AccessExpressionCodegen::INSTANCE;

    /**
     * @brief visit an access expression ast and gen the code
     * @return the value of access expression
     */
    Operand AccessExpressionCodegen::Visit(AccessExpressionNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) {
        //TODO: Add Member Access
        Lexer::Token Identifier = Node.GetIdentifier();

        if (! Program.GetContext().VariableTable.Exists(Identifier.Value)) {
            throw CompilerError(Identifier.Line, Identifier.Column, L"Undefined Identifier!");
        }

        return Program.GetContext().VariableTable.GetSymbol(Identifier.Value).Value;
    }
}