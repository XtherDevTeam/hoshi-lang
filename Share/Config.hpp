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
    using XInteger = int64_t;
    using XDecimal = double;
    using XBoolean = bool;
    using XIndexType = uint64_t;
    using XCharacter = wchar_t;

    template<typename T> using XArray = std::vector<T>;
    template<typename T, typename T1> using XMap = std::unordered_map<T, T1>;

    using XHeapIndexType = XIndexType;
} // Hoshi

#endif //XSCRIPT2_CONFIG_HPP
