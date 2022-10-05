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
        OperandType Type;
        /**
         * @brief Value of the Operand
         */
        XString Value;
        /**
         * @brief List form of the Operand
         */
        XArray<Operand> ListValue;
        /**
         * @brief Mapping form of the Operand
         */
        XTreeMap<XString, Operand> MappingValue;
    public:
        /**
         * @brief Construct a Operand
         * @param Type type of the Operand
         * @param Value value of the Operand
         */
        Operand(OperandType Type, const XString &&Value);

        /**
         * @brief Construct a Operand
         * @param ListValue List form value of the Operand
         */
        Operand(const XArray<Operand> &&ListValue);

        /**
         * @brief Construct a Operand
         * @param MappingValue Mapping form value of the Operand
         */
        Operand(const XTreeMap<XString, Operand> &&MappingValue);

        /**
         * @brief Get the type of the Operand
         * @return Type of the Operand
         */
        OperandType GetType() const;

        /**
         * @brief Get the value of the Operand
         * @return Value of the Operand
         */
        XString GetValue() const;

        /**
         * @brief Get the value of the Operand
         * @return Value of the Operand
         */
        XArray<Operand> GetListValue() const;

        /**
         * @brief Get the value of the Operand
         * @return Value of the Operand
         */
        XTreeMap<XString, Operand> GetMappingValue() const;

        /**
         * @brief Empty Operand
         */
        static Operand Empty;
    };

    XString ToString(Operand Operand);

    XString ToString(XArray<Operand> List);

    XString ToString(XTreeMap<XString, Operand> Mapping);
}

#endif