//
// Created by p0010 on 22-9-11.
//

#include "IR/BinaryExprIR.hpp"
#include "IR/NameUtil.hpp"

#include "LogicEqualExpressionCodegen.hpp"
#include "BinaryExpressionCodegen.hpp"

namespace Hoshi {
    /**
     * @brief Construct a Binary expression codegen
     */
    BinaryExpressionCodegen::BinaryExpressionCodegen(void) = default;

    /**
     * @brief the instance of Binary expression Codegen
     */
    BinaryExpressionCodegen BinaryExpressionCodegen::INSTANCE;

    /**
     * @brief visit an Binary expression ast and gen the code
     * @return the result of Binary expression
     */
    Operand BinaryExpressionCodegen::Visit(BinaryExpressionNode &Node, IRProgram::Builder &Program,
                                               IRBlock::Builder &Block) {
        Operand LastResult = LogicEqualExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(0), Program,
                                                                             Block); //The result of last expression ir
        for (int i = 0; i < Node.GetOperators().size(); i++) {
            Lexer::Token OperatorNode = Node.GetOperators(i);
            Operand ThisOperand = LogicEqualExpressionCodegen::INSTANCE.Visit(*Node.GetOperands(i + 1), Program,
                                                                                  Block); //the operand of this ir
            Operand Result = Operand(OperandType::Identifier,
                                     LocalNamePrefix + NewVarName(L"expr")); // the result of this ir
            Opcode Operator;
            switch (OperatorNode.Kind) { //Choose Opcode
                case Lexer::TokenKind::BinaryAnd:
                    Operator = Opcode::And;
                    break; // '^'
                case Lexer::TokenKind::BinaryOr:
                    Operator = Opcode::Or;
                    break; // '|'
                case Lexer::TokenKind::BinaryXOR:
                    Operator = Opcode::Xor;
                    break; // '^'
                default:
                    throw CompilerError(Node.Line, Node.Column, L"Invalid Binary Expression Operator!");
            }
            Block.AddIR(BinaryExprIR(Operator, LastResult, ThisOperand, Result));
            LastResult = Result;
        }
        return LastResult;
    }
} // Hoshi