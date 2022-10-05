//
// Created by theflysong on 2022/8/29.
//

#include <Codegen/AdditionExpressionCodegen.hpp>
#include <Codegen/MuliplicationExpressionCodegen.hpp>
#include <IR/BinaryExprIR.hpp>
#include <IR/NameUtil.hpp>

namespace Hoshi {
    /**
     * @brief Construct a addition expression codegen
     */
    AdditionExpressionCodegen::AdditionExpressionCodegen(void) = default;

    /**
     * @brief the instance of addition expression Codegen
     */
    AdditionExpressionCodegen AdditionExpressionCodegen::INSTANCE;

    /**
     * @brief visit an addition expression ast and gen the code
     * @return the result of addition expression
     */
    Operand AdditionExpressionCodegen::Visit(AdditionExpressionNode &Node, IRProgram::Builder &Program) {
        IRBlock::Builder &Block = *Program.GetContext().CurrentBlock;
        Operand LastResult = MuliplicationExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(0),
                                                                            Program); //The result of last expression ir
        for (int i = 0; i < Node.GetOperators().size(); i++) {
            Lexer::Token OperatorNode = Node.GetOperators(i);
            Operand ThisOperand = MuliplicationExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(i + 1),
                                                                                 Program); //the operand of this ir
            Operand Result = Operand(OperandType::Identifier,
                                     LocalNamePrefix + NewVarName(L"expr")); // the result of this ir
            Opcode Operator;
            switch (OperatorNode.Kind) { //Choose Opcode
                case Lexer::TokenKind::Plus:
                    Operator = Opcode::Add;
                    break; // '+'
                case Lexer::TokenKind::Minus:
                    Operator = Opcode::Subtract;
                    break; // '-'
                default:
                    throw CompilerError(Node.Line, Node.Column, L"Invalid Addition Expression Operator!");
            }
            Block.AddIR(BinaryExprIR(Operator, LastResult, ThisOperand, Result));
            LastResult = Result;
        }
        return LastResult;
    }
}