//
// Created by theflysong on 2022/8/28.
//

#include <Codegen/LiteralsCodegen.hpp>

namespace Hoshi {
    /**
     * @brief Construct a literals codegen
     */
    LiteralsCodegen::LiteralsCodegen(void) = default;
    /**
     * @brief the instance of literals Codegen
     */
    LiteralsCodegen LiteralsCodegen::INSTANCE;
    /**
     * @brief visit a literal ast and gen the code
     * @return string form of the value of literal
     */
    Operand LiteralsCodegen::Visit(LiteralsNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) {
        switch (Node.GetLiterals().Kind) {
        case Lexer::TokenKind::Integer: return Operand(OperandType::Integer, std::move(Node.GetLiterals().Value));
        case Lexer::TokenKind::Decimal: return Operand(OperandType::Decimal, std::move(Node.GetLiterals().Value));
        case Lexer::TokenKind::Boolean: return Operand(OperandType::Boolean, std::move(Node.GetLiterals().Value));
        case Lexer::TokenKind::Character: return Operand(OperandType::Character, std::move(Node.GetLiterals().Value));
        case Lexer::TokenKind::String: return Operand(OperandType::ReadonlyStringLiteral, std::move(Node.GetLiterals().Value));
        }
        throw CompilerError(Node.Line, Node.Column, L"Invalid Literals!");
    }
}