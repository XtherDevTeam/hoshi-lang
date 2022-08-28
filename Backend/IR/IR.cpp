//
// Created by theflysong on 2022/8/26.
//

#include <IR/IR.hpp>

namespace Hoshi {
    /**
     * @brief Code to String
     * @return String form of the code
     */
    const XString ToString(Opcode Code) {
        switch (Code) {
        case Opcode::NOP     : return L"nop"     ;
        case Opcode::ADD     : return L"add"     ;
        case Opcode::SUB     : return L"sub"     ;
        case Opcode::MUL     : return L"mul"     ;
        case Opcode::DIV     : return L"div"     ;
        case Opcode::REM     : return L"rem"     ;
        case Opcode::SHR     : return L"shr"     ;
        case Opcode::SHL     : return L"shl"     ;
        case Opcode::AND     : return L"and"     ;
        case Opcode::OR      : return L"or"      ;
        case Opcode::XOR     : return L"xor"     ;
        case Opcode::EQU     : return L"equ"     ;
        case Opcode::NEQ     : return L"neq"     ;
        case Opcode::GREAT   : return L"great"   ;
        case Opcode::LESS    : return L"less"    ;
        case Opcode::GREATEQU: return L"greatequ";
        case Opcode::LESSEQU : return L"lessequ" ;
        case Opcode::NOT     : return L"not"     ;
        case Opcode::NEG     : return L"neg"     ;
        case Opcode::INV     : return L"inv"     ;
        case Opcode::JUMP    : return L"jump"    ;
        case Opcode::BRANCH  : return L"branch"  ;
        default              : return L"invalid" ;
        }
    }
    /**
     * @brief Construct an IR
     * @param SourceOperandA First Source Operand
     * @param SourceOperandB Second Source Operand
     * @param DestinationOperand Destination Operand
     */
    IR::IR(const Opcode Code, const Operand SourceOperandA, const Operand SourceOperandB, const Operand DestinationOperand)
        : Code(Code), SourceOperandA(SourceOperandA), SourceOperandB(SourceOperandB), DestinationOperand(DestinationOperand) {
    }
    /**
     * @brief Get the code of the IR
     * @return Code of the IR
     */
    const Opcode IR::GetCode(void) const {
        return Code;
    }
    /**
     * @brief Get the first source Operand of the IR
     * @return First Source Operand of the IR
     */
    const Operand IR::GetSourceOperandA(void) const {
        return SourceOperandA;
    }
    /**
     * @brief Get the second source Operand of the IR
     * @return Second Source Operand of the IR
     */
    const Operand IR::GetSourceOperandB(void) const {
        return SourceOperandB;
    }
    /**
     * @brief Get the destination Operand of the IR
     * @return Destination Operand of the IR
     */
    const Operand IR::GetDestinationOperand(void) const {
        return DestinationOperand;
    }

    /**
     * @brief IR to String
     * @return String form of the ir
     */
    const XString ToString(IR ir) {
        XString str = L"";
        if (ir.DestinationOperand.GetType() == OperandType::Identifier && ir.DestinationOperand.GetValue() != L"") { //是合法的Destination
            str += ir.DestinationOperand.GetValue();
            str += L" = ";
        }
        str += ToString(ir.Code);
        if (ir.SourceOperandA.GetType() != OperandType::Unknown && ir.SourceOperandA.GetValue() != L"") { //是合法的Source
            str += L" ";
            str += ir.SourceOperandA.GetValue();
            if (ir.SourceOperandB.GetType() != OperandType::Unknown && ir.SourceOperandB.GetValue() != L"") { //是合法的Source
                str += L", ";
                str += ir.SourceOperandB.GetValue();
            }
        }
        return str;
    }
}