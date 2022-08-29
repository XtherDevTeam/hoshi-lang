//
// Created by chou on 2022/8/29.
//

#include <Codegen/BinaryMoveExpressionCodegen.hpp>
#include "AdditionExpressionCodegen.hpp"
#include "IR/BinaryExprIR.hpp"
#include "IR/NameUtil.hpp"

namespace Hoshi {
    /**
     * @brief Construct a BinaryMove expression codegen
     */
    BinaryMoveExpressionCodegen::BinaryMoveExpressionCodegen(void) = default;
    /**
     * @brief the instance of BinaryMove expression Codegen
     */
    BinaryMoveExpressionCodegen BinaryMoveExpressionCodegen::INSTANCE;
    /**
     * @brief visit an BinaryMove expression ast and gen the code
     * @return the result of BinaryMove expression
     */
    Operand BinaryMoveExpressionCodegen::Visit(BinaryMoveExpressionNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) {
        Operand LastResult = AdditionExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(0), Program, Block); //The result of last expression ir
        for (int i = 0 ; i < Node.GetOperators().size() ; i ++) {
            Lexer::Token OperatorNode = Node.GetOperators(i);
            Operand ThisOperand = AdditionExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(i + 1), Program, Block); //the operand of this ir
            Operand Result = Operand(OperandType::Identifier, LocalNamePrefix + NewVarName(L"expr")); // the result of this ir
            Opcode Operator;
            switch (OperatorNode.Kind) { //Choose Opcode
            case Lexer::TokenKind::BinaryLeftMove:  Operator = Opcode::ShiftLeft;      break; // '<<'
            case Lexer::TokenKind::BinaryRightMove: Operator = Opcode::ShiftRight; break; // '>>'
            default: throw CompilerError(Node.Line, Node.Column, L"Invalid BinaryMove Expression Operator!");
            }
            Block.AddIR(BinaryExprIR(Operator, LastResult, ThisOperand, Result));
            LastResult = Result;
        }
        return LastResult;
    }
}