//
// Created by theflysong on 2022/8/28.
//

#include <Codegen/UniqueExpressionCodegen.hpp>
#include <Codegen/PrimaryExpressionCodegen.hpp>
#include <IR/UniqueExprIR.hpp>
#include <IR/NameUtil.hpp>

namespace Hoshi {
    /**
     * @brief Construct a unique expression codegen
     */
    UniqueExpressionCodegen::UniqueExpressionCodegen(void) = default;

    /**
     * @brief the instance of unique expression Codegen
     */
    UniqueExpressionCodegen UniqueExpressionCodegen::INSTANCE;

    /**
     * @brief visit an unique expression ast and gen the code
     * @return the result of unique expression
     */
    Operand UniqueExpressionCodegen::Visit(UniqueExpressionNode &Node, IRProgram::Builder &Program) {
        IRBlock::Builder &Block = *Program.GetContext().CurrentBlock;
        Lexer::Token Operator = Node.GetOperator();
        Operand Value = PrimaryExpressionCodegen::INSTANCE.Visit(*Node.GetPrimary(), Program);
        if (Operator == Lexer::Token{} || Operator.Kind == Lexer::TokenKind::Plus) {
            return Value;
        } else if (Operator.Kind == Lexer::TokenKind::Minus) {
            Operand Result = Operand(OperandType::Identifier, LocalNamePrefix + NewVarName(L"expr"));
            Block.AddIR(UniqieExprIR(Opcode::Negative, Value, Result));
            return Result;
        } else if (Operator.Kind == Lexer::TokenKind::Invert) {
            Operand Result = Operand(OperandType::Identifier, LocalNamePrefix + NewVarName(L"expr"));
            Block.AddIR(UniqieExprIR(Opcode::Invert, Value, Result));
            return Result;
        }

        throw CompilerError(Node.Line, Node.Column, L"Invalid Unique Expression Operator!");
    }
}