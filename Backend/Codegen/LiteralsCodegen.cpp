//
// Created by theflysong on 2022/8/27.
//

#include <Codegen/LiteralsCodegen.hpp>
#include <Exceptions/CompilerError.hpp>

namespace Hoshi {
    /**
     * @brief the instance of literals Codegen
     */
    LiteralsCodegen LiteralsCodegen::INSTANCE;

    /**
     * @brief visit an literal ast and gen the code
     * @return string form of the value of literal
     */
    Operand LiteralsCodegen::Visit(AST &ast, IRProgram::Builder &program, IRBlock::Builder &block) {
        if ((! ast.IsLeafNode()) || (ast.Type != AST::TreeType::Literals)) {
            throw CompilerError(ast.Node.Line, ast.Node.Column, L"Wrong ast has been given to the literals codegen!");
        }
        if (ast.Node.Kind == Lexer::TokenKind::Integer) {
            return Operand(OperandType::Integer, std::move(ast.Node.Value));
        }
        if (ast.Node.Kind == Lexer::TokenKind::Decimal) {
            return Operand(OperandType::Decimal, std::move(ast.Node.Value));
        }
        if (ast.Node.Kind == Lexer::TokenKind::Boolean) {
            return Operand(OperandType::Boolean, std::move(ast.Node.Value));
        }
        if (ast.Node.Kind == Lexer::TokenKind::Identifier) {
            return Operand(OperandType::Identifier, std::move(ast.Node.Value));
        }
        if (ast.Node.Kind == Lexer::TokenKind::Character) {
            return Operand(OperandType::Character, std::move(ast.Node.Value));
        }
        if (ast.Node.Kind == Lexer::TokenKind::String) {
            return Operand(OperandType::ReadonlyStringLiteral, std::move(ast.Node.Value));
        }
        return Operand::Empty;
    }
}