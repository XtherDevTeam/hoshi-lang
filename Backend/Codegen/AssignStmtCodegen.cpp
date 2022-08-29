//
// Created by theflysong on 2022/8/29.
//

#include <Codegen/AssignStmtCodegen.hpp>
#include <Codegen/ExpressionCodegen.hpp>
#include <Codegen/AccessExpressionCodegen.hpp>
#include <IR/BinaryExprIR.hpp>
#include <IR/NameUtil.hpp>

namespace Hoshi {
    /**
     * @brief Construct an assign stmt codegen
     */
    AssignStmtCodegen::AssignStmtCodegen(void) = default;
    /**
     * @brief the instance of assign stmt Codegen
     */
    AssignStmtCodegen AssignStmtCodegen::INSTANCE;
    /**
     * @brief visit an assign stmt ast and gen the code
     * @return the value of assign stmt
     */
    Operand AssignStmtCodegen::Visit(AssignStmtNode &Node, IRProgram::Builder &Program, IRBlock::Builder &Block) {
        VariableSymbolInfo &Info = AccessExpressionCodegen::INSTANCE.VisitSymbol(*Node.GetAccess(), Program, Block);
        Operand ExpressionResult = ExpressionCodegen::INSTANCE.Visit(*Node.GetExpression(), Program, Block);

        if (Node.GetAssignOperator().Kind == Lexer::TokenKind::AssignSign) {
            Info.Value = ExpressionResult;
            return Info.Value;
        }
        else if (Info.Value.GetValue() != L"") {
            throw CompilerError(Node.Line, Node.Column, L"Undefined Identifier!");
        }
        else {
            Operand Result = Operand(OperandType::Identifier, LocalNamePrefix + NewVarName(L"expr")); // the result of this ir
            Opcode Operator;
            switch (Node.GetAssignOperator().Kind) {
            case Lexer::TokenKind::RemainderAssignment:      Operator = Opcode::Remainder; break;
            case Lexer::TokenKind::AdditionAssignment:       Operator = Opcode::Add;       break;
            case Lexer::TokenKind::SubtractionAssignment:    Operator = Opcode::Subtract;  break;
            case Lexer::TokenKind::MultiplicationAssignment: Operator = Opcode::Multiply;  break;
            case Lexer::TokenKind::DivisionAssignment:       Operator = Opcode::Divide;    break;
            default: throw CompilerError(Node.Line, Node.Column, L"Invalid Assign Operator!");
            }
            Block.AddIR(BinaryExprIR(Operator, Info.Value, ExpressionResult, Result));
            Info.Value = Result;
            return Info.Value;
        }

        throw CompilerError(Node.Line, Node.Column, L"Invalid Assign Stmt!");
    }
}