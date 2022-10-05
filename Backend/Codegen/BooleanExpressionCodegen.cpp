//
// Created by p0010 on 22-9-23.
//

#include "BooleanExpressionCodegen.hpp"

#include "IR/NameUtil.hpp"

#include "BinaryExpressionCodegen.hpp"
#include "IR/BinaryExprIR.hpp"

namespace Hoshi {
    /**
     * @brief Construct a Boolean expression codegen
     */
    BooleanExpressionCodegen::BooleanExpressionCodegen(void) = default;

    /**
     * @brief the instance of Boolean expression Codegen
     */
    BooleanExpressionCodegen BooleanExpressionCodegen::INSTANCE;

    /**
     * @brief visit an Boolean expression ast and gen the code
     * @return the result of Boolean expression
     */
    Operand BooleanExpressionCodegen::Visit(BooleanExpressionNode &Node, IRProgram::Builder &Program) {
        IRBlock::Builder &Block = *Program.GetContext().CurrentBlock;
        Operand LastResult = BinaryExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(0),
                                                                     Program); //The result of last expression ir
        for (int i = 0; i < Node.GetOperators().size(); i++) {
            Lexer::Token OperatorNode = Node.GetOperators(i);
            Operand ThisOperand = BinaryExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(i + 1),
                                                                          Program); //the operand of this ir
            Operand Result = Operand(OperandType::Identifier,
                                     LocalNamePrefix + NewVarName(L"expr")); // the result of this ir
            Opcode Operator;
            switch (OperatorNode.Kind) { //Choose Opcode
                case Lexer::TokenKind::LogicAnd:
                    Operator = Opcode::And;
                    break; // '^'
                case Lexer::TokenKind::LogicOr:
                    Operator = Opcode::Or;
                    break; // '|'
                default:
                    throw CompilerError(Node.Line, Node.Column, L"Invalid Boolean Expression Operator!");
            }
            Block.AddIR(BinaryExprIR(Operator, LastResult, ThisOperand, Result));
            LastResult = Result;
        }
        return LastResult;
    }
} // Hoshi