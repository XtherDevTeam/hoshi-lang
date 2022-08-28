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
    Operand SingleExpressionCodegen::Visit(AST &ast, IRProgram::Builder &Program, IRBlock::Builder &Block) {
        if (ast.Type != AST::TreeType::SingleExpression) {
            return PrimaryCodegen::INSTANCE.Visit(ast, Program, Block);
        }
        
        Lexer::Token Operator = ast.GetFirstNotNullToken();
        Operand value = PrimaryCodegen::INSTANCE.Visit(ast.Subtrees[1], Program, Block);
        if (Operator.Kind == Lexer::TokenKind::Plus) {
            return value;
        }
        else if (Operator.Kind == Lexer::TokenKind::Minus) {
            Operand result = Operand(OperandType::Identifier, LocalNamePrefix + NewVarName(L"expr"));
            Block.AddIR(UniqieExprIR(Opcode::NEG, value, result));
            return result;
        }
        else if (Operator.Kind == Lexer::TokenKind::Invert) {
            Operand Result = Operand(OperandType::Identifier, LocalNamePrefix + NewVarName(L"expr"));
            Block.AddIR(UniqieExprIR(Opcode::INV, value, Result));
            return Result;
        }
        return Operand::Empty;
    }
}