//
// Created by Jerry Chou on 2022/5/7.
//

#ifndef XSCRIPT2_UTILS_HPP
#define XSCRIPT2_UTILS_HPP

#include <cstring>
#include <mutex>
#include <filesystem>

#include "Config.hpp"

namespace Hoshi {
    extern XIndexType CreatedUnfreeElement;

    extern std::mutex InterpreterLock;

    std::string wstring2string(const std::wstring &ws);

    std::wstring string2wstring(const std::string &s);

    std::wstring unescape_string(const std::wstring &s);

    bool IsDigit(wchar_t Char);

    bool IsAlpha(wchar_t Char);

    bool IsHexDigit(wchar_t Char);

    wchar_t ToHexDigit(wchar_t Char);

    XInteger XStringToXInteger(const XString &T);

    XDecimal XStringToXDecimal(const XString &T);

    XIndexType HashBytes(const XBytes& Bytes);

    XIndexType Hash(const XString &T);

    /**
     * Merge R into the back of L
     * @tparam T Original object
     * @param R The Object to merge
     */
    template<typename T>
    void MergeArray(XArray<T> &L, const XArray<T> &R) {
        for (auto &Element: R) {
            L.push_back(Element);
        }
    }

    XArray<XString> SplitStrings(const XString &Str, XCharacter Delim);
} // Hoshi

#endif //XSCRIPT2_UTILS_HPP
