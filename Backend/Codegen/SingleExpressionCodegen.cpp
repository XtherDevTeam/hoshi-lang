//
// Created by theflysong on 2022/8/27.
//

#include <Codegen/SingleExpressionCodegen.hpp>
#include <Codegen/PrimaryCodegen.hpp>
#include <Exceptions/CompilerError.hpp>
#include <IR/NameUtil.hpp>
#include <IR/UniqueExprIR.hpp>

namespace Hoshi {
    /**
     * @brief the instance of literals Codegen
     */
    SingleExpressionCodegen SingleExpressionCodegen::INSTANCE;

    /**
     * @brief visit an literal ast and gen the code
     * @return string form of the value of literal
     */
    Operand SingleExpressionCodegen::Visit(AST &ast, IRProgram::Builder &program, IRBlock::Builder &block) {
        if (ast.Type != AST::TreeType::SingleExpression) {
            throw CompilerError(ast.Node.Line, ast.Node.Column, L"Wrong ast has been given to the single expression codegen!");
        }
        
        Lexer::Token Operator = ast.GetFirstNotNullToken();
        Operand value = PrimaryCodegen::INSTANCE.Visit(ast.Subtrees[1], program, block);
        if (Operator.Kind == Lexer::TokenKind::Plus) {
            return value;
        }
        else if (Operator.Kind == Lexer::TokenKind::Minus) {
            Operand result = Operand(OperandType::Identifier, LocalNamePrefix + NewVarName(L"expr"));
            block.AddIR(UniqieExprIR(Opcode::NEG, value, result));
            return result;
        }
        else if (Operator.Kind == Lexer::TokenKind::Invert) {
            Operand result = Operand(OperandType::Identifier, LocalNamePrefix + NewVarName(L"expr"));
            block.AddIR(UniqieExprIR(Opcode::INV, value, result));
            return result;
        }
        return Operand::Empty;
    }
}