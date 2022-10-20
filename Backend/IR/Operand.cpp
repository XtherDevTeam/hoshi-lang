//
// Created by theflysong on 2022/8/26.
//

#include <IR/Operand.hpp>

namespace Hoshi {
    /**
     * @brief Construct a Operand
     */
    Operand::Operand(const OperandType Type, const XString &&Value)
            : Type(Type), Value(Value), ListValue(), MappingValue() {
    }

    /**
     * @brief Construct a Operand
     */
    Operand::Operand(const XArray<Operand> &&ListValue)
            : Type(OperandType::List), ListValue(ListValue), MappingValue() {

    }

    /**
     * @brief Construct a Operand
     */
    Operand::Operand(const XTreeMap<XString, Operand> &&MappingValue)
            : Type(OperandType::Mapping), ListValue(), MappingValue(MappingValue) {
    }

    /**
     * @brief Get the value of the Operand
     * @return Value of the Operand
     */
    XString Operand::GetValue() const {
        return Value;
    }

    /**
     * @brief Get the value of the Operand
     * @return Value of the Operand
     */
    XArray<Operand> Operand::GetListValue() const {
        return ListValue;
    }

    /**
     * @brief Get the value of the Operand
     * @return Value of the Operand
     */
    XTreeMap<XString, Operand> Operand::GetMappingValue() const {
        return MappingValue;
    }

    /**
     * @brief Get the type of the Operand
     * @return Type of the Operand
     */
    OperandType Operand::GetType() const {
        return Type;
    }

    /**
     * @brief Empty Operand
     */
    Operand Operand::Empty(OperandType::Unknown, L"");

    XString ToString(Operand Operand) {
        if (Operand.GetType() == OperandType::Mapping) {
            return ToString(Operand.GetMappingValue());
        }
        if (Operand.GetType() == OperandType::List) {
            return ToString(Operand.GetListValue());
        }
        return Operand.GetValue();
    }

    XString ToString(XArray<Operand> List) {
        XString Str = L"[";
        for (const Operand& Op: List) {
            Str += ToString(Op);
            Str += L",";
        }
        Str[Str.length() - 1] = L']';
        return Str;
    }

    XString ToString(XTreeMap<XString, Operand> Mapping) {
        XString Str = L"{";
        for (const auto& pair: Mapping) {
            Str += pair.first;
            Str += L":";
            Str += ToString(pair.second);
            Str += L",";
        }
        Str[Str.length() - 1] = L'}';
        return Str;
    }
} // Hoshi
