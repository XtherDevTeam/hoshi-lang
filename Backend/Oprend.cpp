//
// Created by theflysong on 2022/8/26.
//

#include <Oprend.hpp>

namespace Hoshi {
    /**
     * @brief Construct a Oprend
     */
    Oprend::Oprend(const OprendType Type, const std::string &&Value)
        : Type(Type), Value(std::move(Value)), ListValue(), MappingValue()
    {
    }
    /**
     * @brief Construct a Oprend
     */
    Oprend::Oprend(const std::vector<Oprend> &&ListValue)
        : Type(OprendType::List), Value(""), ListValue(std::move(ListValue)), MappingValue() {

    }
    /**
     * @brief Construct a Oprend
     */
    Oprend::Oprend(const std::map<std::string, Oprend> &&MappingValue)
        : Type(OprendType::Mapping), Value(""), ListValue(), MappingValue(std::move(MappingValue)) {
    }
    /**
     * @brief Get the value of the Oprend
     * @return Value of the Oprend
     */
    const std::string Oprend::GetValue(void) const {
        return Value;
    }
    /**
     * @brief Get the value of the Oprend
     * @return Value of the Oprend
     */
    const std::vector<Oprend> Oprend::GetListValue(void) const {
        return ListValue;
    }
    /**
     * @brief Get the value of the Oprend
     * @return Value of the Oprend
     */
    const std::map<std::string, Oprend> Oprend::GetMappingValue(void) const {
        return MappingValue;
    }
    /**
     * @brief Get the type of the Oprend
     * @return Type of the Oprend
     */
    const OprendType Oprend::GetType(void) const {
        return Type;
    }
    /**
     * @brief Empty Oprend
     */
    Oprend Oprend::Empty(OprendType::Unknown, "");
} // Hoshi
