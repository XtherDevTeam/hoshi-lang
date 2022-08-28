//
// Created by theflysong on 2022/8/28.
//

#include <Codegen/AdditionExpressionCodegen.hpp>
#include <Codegen/MultiplicationExpressionCodegen.hpp>
#include <Exceptions/CompilerError.hpp>
#include <IR/BinaryExprIR.hpp>
#include <IR/NameUtil.hpp>

namespace Hoshi {
    /**
     * @brief the instance of literals Codegen
     */
    AdditionExpressionCodegen AdditionExpressionCodegen::INSTANCE;

    /**
     * @brief visit an literal ast and gen the code
     * @return string form of the value of literal
     */
    Operand AdditionExpressionCodegen::Visit(AST &ast, IRProgram::Builder &Program, IRBlock::Builder &Block) {
        if (ast.Type != AST::TreeType::AdditionExpression) {
            return MultiplicationExpressionCodegen::INSTANCE.Visit(ast, Program, Block);
        }

        Operand FirstOperand = AdditionExpressionCodegen::INSTANCE.Visit(ast.Subtrees[0], Program, Block);
        Lexer::Token OperatorNode = ast.Subtrees[1].Node;
        Operand SecondOperand = MultiplicationExpressionCodegen::INSTANCE.Visit(ast.Subtrees[2], Program, Block);
        Opcode Operator;
        switch (OperatorNode.Kind) {
        case Lexer::TokenKind::Plus: Operator = Opcode::ADD; break;
        case Lexer::TokenKind::Minus: Operator = Opcode::SUB; break;
        default: throw CompilerError(ast.Node.Line, ast.Node.Column, L"Wrong operator has been given to the multiplication expression codegen!");
        }
        Operand Result = Operand(OperandType::Identifier, LocalNamePrefix + NewVarName(L"expr"));
        Block.AddIR(BinaryExprIR(Operator, FirstOperand, SecondOperand, Result));
        return Result;
    }
}