//
// Created by theflysong on 2022/8/26.
//

#ifndef XSCRIPT2_OPREND_HPP
#define XSCRIPT2_OPREND_HPP

#include <string>
#include <map>
#include <vector>

namespace Hoshi {
    enum class OprendType : int {
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

    class Oprend {
        /**
         * @brief Type of the Oprend
         */
        const OprendType Type;
        /**
         * @brief Value of the Oprend
         */
        const std::string Value;
        /**
         * @brief List form of the Oprend
         */
        const std::vector<Oprend> ListValue;
        /**
         * @brief Mapping form of the Oprend
         */
        const std::map<std::string, Oprend> MappingValue;
    public:
        /**
         * @brief Construct a Oprend
         * @param Type type of the oprend
         * @param Value value of the oprend
         */
        Oprend(const OprendType Type, const std::string &&Value);
        /**
         * @brief Construct a Oprend
         * @param ListValue List form value of the oprend
         */
        Oprend(const std::vector<Oprend> &&ListValue);
        /**
         * @brief Construct a Oprend
         * @param MappingValue Mapping form value of the oprend
         */
        Oprend(const std::map<std::string, Oprend> &&MappingValue);
        /**
         * @brief Get the type of the Oprend
         * @return Type of the Oprend
         */
        const OprendType GetType(void) const;
        /**
         * @brief Get the value of the Oprend
         * @return Value of the Oprend
         */
        const std::string GetValue(void) const;
        /**
         * @brief Get the value of the Oprend
         * @return Value of the Oprend
         */
        const std::vector<Oprend> GetListValue(void) const;
        /**
         * @brief Get the value of the Oprend
         * @return Value of the Oprend
         */
        const std::map<std::string, Oprend> GetMappingValue(void) const;
        /**
         * @brief Empty Oprend
         */
        static Oprend Empty;
    };
}

#endif