//
// Created by XIaokang00010 on 2023/2/10.
//

#include <share/def.hpp>

namespace yoi {
    void parseString(std::wistream &input, wstr &value) {
        wchar ch = '\0';
        while (input) {
            if (!input.get(ch)) break;
            if (ch == '\\') {
                if (!input.get(ch)) break;
                switch (ch) {
                    case '\\':
                    case '"':
                    case '\'':
                    case '/':
                        value.push_back(ch);
                        break;
                    case 'b':
                        value.push_back('\b');
                        break;
                    case 'f':
                        value.push_back('\f');
                        break;
                    case 'n':
                        value.push_back('\n');
                        break;
                    case 'r':
                        value.push_back('\r');
                        break;
                    case 't':
                        value.push_back('\t');
                        break;
                    case 'u': {
                        wchar fuckutf{};
                        for (int64_t i = 3; input && i >= 0; i--) {
                            if (!input.get(ch)) break;
                            if ('a' <= ch and ch <= 'z')
                                fuckutf += ((ch - 'a' + 10) * (1 << 4 * i));
                            else if ('A' <= ch and ch <= 'Z')
                                fuckutf += ((ch - 'A' + 10) * (1 << 4 * i));
                            else
                                fuckutf += ((ch - '0') * (1 << 4 * i));
                        }
                        value.push_back(fuckutf);
                        break;
                    }
                    default:
                        value.push_back(ch);
                        break;
                }
            } else {
                value.push_back(ch);
            }
        }

    }

    void panic(int64_t line, int64_t col, const std::string &msg) {
        throw std::runtime_error("At line " + std::to_string(line) + " col " + std::to_string(col) + ": " + msg);
    }

    void assert(bool condition, int64_t line, int64_t col, const std::string &msg) {
        if (not condition) {
            throw std::runtime_error("At line " + std::to_string(line) + " col " + std::to_string(col) + ": " + msg);
        }
    }

    std::wstring string2wstring(const std::string &v) {
        std::wstring result;
        utf8Unicode::utf8ToUnicode(v, result);
        return result;
    }

    std::string wstring2string(const std::wstring &v) {
        std::string result;
        utf8Unicode::unicodeToUtf8(v, result);
        return result;
    }
}