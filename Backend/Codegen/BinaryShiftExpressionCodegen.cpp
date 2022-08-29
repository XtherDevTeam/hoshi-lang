//
// Created by chou on 2022/8/29.
//

#include <Codegen/BinaryShiftExpressionCodegen.hpp>
#include "AdditionExpressionCodegen.hpp"
#include "IR/BinaryExprIR.hpp"
#include "IR/NameUtil.hpp"

namespace Hoshi {
    /**
     * @brief Construct a BinaryShift expression codegen
     */
    BinaryShiftExpressionCodegen::BinaryShiftExpressionCodegen(void) = default;
    /**
     * @brief the instance of BinaryShift expression Codegen
     */
    BinaryShiftExpressionCodegen BinaryShiftExpressionCodegen::INSTANCE;
    /**
     * @brief visit an BinaryShift expression ast and gen the code
     * @return the result of BinaryShift expression
     */
    Operand BinaryShiftExpressionCodegen::Visit(BinaryShiftExpressionNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) {
        Operand LastResult = AdditionExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(0), Program, Block); //The result of last expression ir
        for (int i = 0 ; i < Node.GetOperators().size() ; i ++) {
            Lexer::Token OperatorNode = Node.GetOperators(i);
            Operand ThisOperand = AdditionExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(i + 1), Program, Block); //the operand of this ir
            Operand Result = Operand(OperandType::Identifier, LocalNamePrefix + NewVarName(L"expr")); // the result of this ir
            Opcode Operator;
            switch (OperatorNode.Kind) { //Choose Opcode
            case Lexer::TokenKind::BinaryLeftShift:  Operator = Opcode::ShiftLeft;      break; // '<<'
            case Lexer::TokenKind::BinaryRightShift: Operator = Opcode::ShiftRight; break; // '>>'
            default: throw CompilerError(Node.Line, Node.Column, L"Invalid BinaryShift Expression Operator!");
            }
            Block.AddIR(BinaryExprIR(Operator, LastResult, ThisOperand, Result));
            LastResult = Result;
        }
        return LastResult;
    }
}