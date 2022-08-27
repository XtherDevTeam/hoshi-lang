//
// Created by theflysong on 2022/8/26.
//

#include <IR/Operand.hpp>

namespace Hoshi {
    /**
     * @brief Construct a Operand
     */
    Operand::Operand(const OperandType Type, const XString &&Value)
        : Type(Type), Value(std::move(Value)), ListValue(), MappingValue()
    {
    }
    /**
     * @brief Construct a Operand
     */
    Operand::Operand(const XArray<Operand> &&ListValue)
        : Type(OperandType::List), Value(L""), ListValue(std::move(ListValue)), MappingValue() {

    }
    /**
     * @brief Construct a Operand
     */
    Operand::Operand(const XTreeMap<XString, Operand> &&MappingValue)
        : Type(OperandType::Mapping), Value(L""), ListValue(), MappingValue(std::move(MappingValue)) {
    }
    /**
     * @brief Get the value of the Operand
     * @return Value of the Operand
     */
    const XString Operand::GetValue(void) const {
        return Value;
    }
    /**
     * @brief Get the value of the Operand
     * @return Value of the Operand
     */
    const XArray<Operand> Operand::GetListValue(void) const {
        return ListValue;
    }
    /**
     * @brief Get the value of the Operand
     * @return Value of the Operand
     */
    const XTreeMap<XString, Operand> Operand::GetMappingValue(void) const {
        return MappingValue;
    }
    /**
     * @brief Get the type of the Operand
     * @return Type of the Operand
     */
    const OperandType Operand::GetType(void) const {
        return Type;
    }
    /**
     * @brief Empty Operand
     */
    Operand Operand::Empty(OperandType::Unknown, L"");
    const XString ToString(Operand Operand) {
        if (Operand.GetType() == OperandType::Mapping) {
            return ToString(Operand.GetMappingValue());
        }
        if (Operand.GetType() == OperandType::List) {
            return ToString(Operand.GetListValue());
        }
        return Operand.GetValue();
    }
    const XString ToString(XArray<Operand> List) {
        XString Str = L"[";
        for (Operand Op : List) {
            Str += ToString(Op);
            Str += L",";
        }
        Str[Str.length() - 1] = L']';
        return Str;
    }
    const XString ToString(XTreeMap<XString, Operand> Mapping) {
        XString Str = L"{";
        for (auto pair : Mapping) {
            Str += pair.first;
            Str += L":";
            Str += ToString(pair.second);
            Str += L",";
        }
        Str[Str.length() - 1] = L'}';
        return Str;
    }
} // Hoshi
