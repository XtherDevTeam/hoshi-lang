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
        case Opcode::Nop       : return L"nop"       ;
        case Opcode::Add       : return L"add"       ;
        case Opcode::Subtract  : return L"subtract"  ;
        case Opcode::Multiply  : return L"multiply"  ;
        case Opcode::Divide    : return L"divide"    ;
        case Opcode::Remainder : return L"remainder" ;
        case Opcode::ShiftRight: return L"shiftright";
        case Opcode::ShiftLeft : return L"shiftleft" ;
        case Opcode::And       : return L"and"       ;
        case Opcode::Or        : return L"or"        ;
        case Opcode::Xor       : return L"xor"       ;
        case Opcode::Equal     : return L"equal"     ;
        case Opcode::NotEqual  : return L"notequal"  ;
        case Opcode::Great     : return L"great"     ;
        case Opcode::Less      : return L"less"      ;
        case Opcode::GreatEqual: return L"greatequal";
        case Opcode::LessEqual : return L"lessequ"   ;
        case Opcode::Not       : return L"not"       ;
        case Opcode::Negative  : return L"negative"  ;
        case Opcode::Invert    : return L"invert"    ;
        case Opcode::Jump      : return L"jump"      ;
        case Opcode::Branch    : return L"branch"    ;
        default                : return L"invalid"   ;
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