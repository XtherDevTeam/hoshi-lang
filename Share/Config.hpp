//
// Created by Jerry Chou on 2022/5/7.
//

#ifndef XSCRIPT2_CONFIG_HPP
#define XSCRIPT2_CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <clocale>
#include <set>

namespace Hoshi {
    using XBytes = std::string;
    using XString = std::wstring;
    using XByte = uint8_t;
    using XShort = int16_t;
    using XUShort = uint16_t;
    using XShortInt = int32_t;
    using XUShortInt = uint32_t;
    using XInteger = int64_t;
    using XUInteger = uint64_t;
    using XReal = double;
    using XFloat = float;
    using XDecimal = XReal;
    using XBoolean = bool;
    using XIndexType = XUInteger;
    using XCharacter = wchar_t;
    using XShortChar = char;
    using XSize = XUInteger;

    template<typename T> using XArray = std::vector<T>;
    template<typename T, typename T1> using XMap = std::unordered_map<T, T1>;
    template<typename T, typename T1> using XTreeMap = std::map<T, T1>;
    template<typename T, typename T1> using XPair = std::pair<T, T1>;

    using XHeapIndexType = XIndexType;
} // Hoshi

#endif //XSCRIPT2_CONFIG_HPP
