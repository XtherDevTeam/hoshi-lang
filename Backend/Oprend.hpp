//
// Created by theflysong on 2022/8/26.
//

#ifndef XSCRIPT2_OPREND_HPP
#define XSCRIPT2_OPREND_HPP

#include <string>

namespace Hoshi {
    enum class OprendType : int {
        Unknown = 0,
        Identifier,
        Integer,
        Decimal,
        Boolean,
        Character,
        ReadonlyStringLiteral
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
    public:
        /**
         * @brief Construct a Oprend
         */
        Oprend(const OprendType Type, const std::string &&Value);
        /**
         * @brief Get the value of the Oprend
         * @return Value of the Oprend
         */
        const std::string GetValue(void) const;
        /**
         * @brief Get the type of the Oprend
         * @return Type of the Oprend
         */
        const OprendType GetType(void) const;
        /**
         * @brief Empty Oprend
         */
        static Oprend Empty;
    };
}

#endif