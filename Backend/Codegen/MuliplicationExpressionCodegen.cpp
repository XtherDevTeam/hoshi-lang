//
// Created by theflysong on 2022/8/29.
//

#include <Codegen/MuliplicationExpressionCodegen.hpp>
#include <Codegen/UniqueExpressionCodegen.hpp>
#include <IR/BinaryExprIR.hpp>
#include <IR/NameUtil.hpp>

namespace Hoshi {
    /**
     * @brief Construct a muliplication expression codegen
     */
    MuliplicationExpressionCodegen::MuliplicationExpressionCodegen(void) = default;
    /**
     * @brief the instance of muliplication expression Codegen
     */
    MuliplicationExpressionCodegen MuliplicationExpressionCodegen::INSTANCE;
    /**
     * @brief visit an muliplication expression ast and gen the code
     * @return the result of muliplication expression
     */
    Operand MuliplicationExpressionCodegen::Visit(MuliplicationExpressionNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) {
        Operand LastResult = UniqueExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(0), Program, Block); //The result of last expression ir
        for (int i = 0 ; i < Node.GetOperators().size() ; i ++) {
            Lexer::Token OperatorNode = Node.GetOperators(i);
            Operand ThisOperand = UniqueExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(i + 1), Program, Block); //the operand of this ir
            Operand Result = Operand(OperandType::Identifier, LocalNamePrefix + NewVarName(L"expr")); // the result of this ir
            Opcode Operator;
            switch (OperatorNode.Kind) { //Choose Opcode
            case Lexer::TokenKind::Asterisk:    Operator = Opcode::Multiply;  break; // '*'
            case Lexer::TokenKind::Slash:       Operator = Opcode::Divide;    break; // '/'
            case Lexer::TokenKind::PercentSign: Operator = Opcode::Remainder; break; // '%'
            default: throw CompilerError(Node.Line, Node.Column, L"Invalid Muliplication Expression Operator!");
            }
            Block.AddIR(BinaryExprIR(Operator, LastResult, ThisOperand, Result));
            LastResult = Result;
        }
        return LastResult;
    }
}