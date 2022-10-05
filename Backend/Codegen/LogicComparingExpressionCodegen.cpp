//
// Created by chou on 8/30/22.
//

#include "LogicComparingExpressionCodegen.hpp"
#include "BinaryShiftExpressionCodegen.hpp"

#include "IR/BinaryExprIR.hpp"
#include "IR/NameUtil.hpp"

namespace Hoshi {
    /**
     * @brief Construct a LogicComparing expression codegen
     */
    LogicComparingExpressionCodegen::LogicComparingExpressionCodegen(void) = default;

    /**
     * @brief the instance of LogicComparing expression Codegen
     */
    LogicComparingExpressionCodegen LogicComparingExpressionCodegen::INSTANCE;

    /**
     * @brief visit an LogicComparing expression ast and gen the code
     * @return the result of LogicComparing expression
     */
    Operand LogicComparingExpressionCodegen::Visit(LogicComparingExpressionNode &Node, IRProgram::Builder &Program) {
        IRBlock::Builder &Block = *Program.GetContext().CurrentBlock;
        Operand LastResult = BinaryShiftExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(0),
                                                                          Program); //The result of last expression ir
        for (int i = 0; i < Node.GetOperators().size(); i++) {
            Lexer::Token OperatorNode = Node.GetOperators(i);
            Operand ThisOperand = BinaryShiftExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(i + 1),
                                                                               Program); //the operand of this ir
            Operand Result = Operand(OperandType::Identifier,
                                     LocalNamePrefix + NewVarName(L"expr")); // the result of this ir
            Opcode Operator;
            switch (OperatorNode.Kind) { //Choose Opcode
                case Lexer::TokenKind::LessThan:
                    Operator = Opcode::Less;
                    break; // '<'
                case Lexer::TokenKind::LessThanOrEqual:
                    Operator = Opcode::LessEqual;
                    break; // '<='
                case Lexer::TokenKind::GreaterThan:
                    Operator = Opcode::Great;
                    break; // '>'
                case Lexer::TokenKind::GreaterThanOrEqual:
                    Operator = Opcode::GreatEqual;
                    break; // '>='
                default:
                    throw CompilerError(Node.Line, Node.Column, L"Invalid LogicComparing Expression Operator!");
            }
            Block.AddIR(BinaryExprIR(Operator, LastResult, ThisOperand, Result));
            LastResult = Result;
        }
        return LastResult;
    }
} // Hoshi