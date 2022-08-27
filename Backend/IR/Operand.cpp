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
    Operand::Operand(const std::vector<Operand> &&ListValue)
        : Type(OperandType::List), Value(L""), ListValue(std::move(ListValue)), MappingValue() {

    }
    /**
     * @brief Construct a Operand
     */
    Operand::Operand(const std::map<XString, Operand> &&MappingValue)
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
    const std::vector<Operand> Operand::GetListValue(void) const {
        return ListValue;
    }
    /**
     * @brief Get the value of the Operand
     * @return Value of the Operand
     */
    const std::map<XString, Operand> Operand::GetMappingValue(void) const {
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
} // Hoshi
