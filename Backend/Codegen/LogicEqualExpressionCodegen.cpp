//
// Created by chou on 8/31/22.
//

#include "IR/BinaryExprIR.hpp"
#include "IR/NameUtil.hpp"

#include "LogicEqualExpressionCodegen.hpp"
#include "LogicComparingExpressionCodegen.hpp"

namespace Hoshi {
    /**
     * @brief Construct a LogicEqual expression codegen
     */
    LogicEqualExpressionCodegen::LogicEqualExpressionCodegen(void) = default;

    /**
     * @brief the instance of LogicEqual expression Codegen
     */
    LogicEqualExpressionCodegen LogicEqualExpressionCodegen::INSTANCE;

    /**
     * @brief visit an LogicEqual expression ast and gen the code
     * @return the result of LogicEqual expression
     */
    Operand LogicEqualExpressionCodegen::Visit(LogicEqualExpressionNode &Node, IRProgram::Builder &Program) {
        IRBlock::Builder &Block = *Program.GetContext().CurrentBlock;
        Operand LastResult = LogicComparingExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(0),
                                                                             Program); //The result of last expression ir
        for (int i = 0; i < Node.GetOperators().size(); i++) {
            Lexer::Token OperatorNode = Node.GetOperators(i);
            Operand ThisOperand = LogicComparingExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(i + 1),
                                                                                  Program); //the operand of this ir
            Operand Result = Operand(OperandType::Identifier,
                                     LocalNamePrefix + NewVarName(L"expr")); // the result of this ir
            Opcode Operator;
            switch (OperatorNode.Kind) { //Choose Opcode
                case Lexer::TokenKind::Equal:
                    Operator = Opcode::Equal;
                    break; // '=='
                case Lexer::TokenKind::NotEqual:
                    Operator = Opcode::NotEqual;
                    break; // '!='
                default:
                    throw CompilerError(Node.Line, Node.Column, L"Invalid LogicEqual Expression Operator!");
            }
            Block.AddIR(BinaryExprIR(Operator, LastResult, ThisOperand, Result));
            LastResult = Result;
        }
        return LastResult;
    }
} // Hoshi