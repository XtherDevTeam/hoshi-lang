//
// Created by theflysong on 2022/8/26.
//

#ifndef XSCRIPT2_Operand_HPP
#define XSCRIPT2_Operand_HPP

#include <string>
#include <map>
#include <vector>
#include <Config.hpp>

namespace Hoshi {
    enum class OperandType : int {
        Unknown = 0,
        Identifier,
        Integer,
        Decimal,
        Boolean,
        Character,
        ReadonlyStringLiteral,
        Label,
        List,
        Mapping
    };

    class Operand {
        /**
         * @brief Type of the Operand
         */
        const OperandType Type;
        /**
         * @brief Value of the Operand
         */
        const XString Value;
        /**
         * @brief List form of the Operand
         */
        const std::vector<Operand> ListValue;
        /**
         * @brief Mapping form of the Operand
         */
        const std::map<XString, Operand> MappingValue;
    public:
        /**
         * @brief Construct a Operand
         * @param Type type of the Operand
         * @param Value value of the Operand
         */
        Operand(const OperandType Type, const XString &&Value);
        /**
         * @brief Construct a Operand
         * @param ListValue List form value of the Operand
         */
        Operand(const std::vector<Operand> &&ListValue);
        /**
         * @brief Construct a Operand
         * @param MappingValue Mapping form value of the Operand
         */
        Operand(const std::map<XString, Operand> &&MappingValue);
        /**
         * @brief Get the type of the Operand
         * @return Type of the Operand
         */
        const OperandType GetType(void) const;
        /**
         * @brief Get the value of the Operand
         * @return Value of the Operand
         */
        const XString GetValue(void) const;
        /**
         * @brief Get the value of the Operand
         * @return Value of the Operand
         */
        const std::vector<Operand> GetListValue(void) const;
        /**
         * @brief Get the value of the Operand
         * @return Value of the Operand
         */
        const std::map<XString, Operand> GetMappingValue(void) const;
        /**
         * @brief Empty Operand
         */
        static Operand Empty;
    };
}

#endif