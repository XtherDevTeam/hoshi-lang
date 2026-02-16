//
// Created by XIaokang00010 on 2023/2/10.
//

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <share/def.hpp>
#include <stdlib.h>

namespace yoi {
    thread_local yoi::wstr __current_file_path = L"";
    std::mutex consoleMutex;

    std::map<std::string, ExceptionHandleType> exception_categories = {
        {"NULLABLE_VALUE_SUPPLY_TO_RAW", ExceptionHandleType::Suppress},
        {"INTERNAL", ExceptionHandleType::Panic},
        {"UCRT_NOT_FOUND", ExceptionHandleType::Warning},
        {"ELYSIA_RUNTIME_NOT_FOUND", ExceptionHandleType::Warning},
        {"MODULE_NOT_MODIFIED", ExceptionHandleType::Suppress},
    };

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
                    case 'e':
                        value.push_back('\033');
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
                    case '0':
                        value.push_back('\0');
                        break;
                    default:
                        value.push_back(ch);
                        break;
                }
            } else {
                value.push_back(ch);
            }
        }

    }

    wstr escapeString(const wstr &value) {
        wstr result;
        for (auto ch : value) {
            switch (ch) {
                case L'\\': result += L"\\\\"; break;
                case L'\"': result += L"\\\""; break;
                case L'\'': result += L"\\\'"; break;
                case L'\b': result += L"\\b"; break;
                case L'\f': result += L"\\f"; break;
                case L'\n': result += L"\\n"; break;
                case L'\r': result += L"\\r"; break;
                case L'\t': result += L"\\t"; break;
                case L'\033': result += L"\\e"; break;
                case L'\0': result += L"\\0"; break;
                default:
                    if (ch < 32 || ch > 126) {
                        wchar_t buf[7];
                        swprintf(buf, 7, L"\\u%04x", (unsigned int)ch);
                        result += buf;
                    } else {
                        result += ch;
                    }
                    break;
            }
        }
        return result;
    }


    void set_current_file_path(const std::wstring &path) {
        __current_file_path = path;
    }
    
    std::wstring get_line_hint_for_error(const std::wstring &file, yoi::indexT line, yoi::indexT col) {
        std::fstream fileStream(yoi::wstring2string(file), std::ios::in);
        if (!fileStream.is_open()) {
            return L"";
        }
        std::string lineStr;
        for (yoi::indexT i = 0; i < line; i++) {
            std::getline(fileStream, lineStr);
        }
        std::wstring result = yoi::string2wstring(lineStr) + L"\n";
        result += std::wstring(col - 1, ' ') + L"^";
        return result;
    }

    void panic(yoi::indexT line, yoi::indexT col, const std::string &msg) {
        auto message =  msg;
        if (!__current_file_path.empty()) {
            message += " near " + yoi::wstring2string(__current_file_path) + ":" + std::to_string(line + 1) + ":" + std::to_string(col + 1);
            message += "\n" + yoi::wstring2string(get_line_hint_for_error(__current_file_path, line + 1, col + 1));
        } else {
            message += " near line " + std::to_string(line) + " col " + std::to_string(col);
        }

        throw std::runtime_error(message);
    }

    void warning(yoi::indexT line, yoi::indexT col, const std::string& msg, const std::string& label) {
        if (exception_categories.find(label) == exception_categories.end() || exception_categories[label] == ExceptionHandleType::Suppress) {
            return;
        }
        if (exception_categories[label] == ExceptionHandleType::Panic) {
            panic(line, col, msg);
        }

        auto message =  msg;
        if (!__current_file_path.empty()) {
            message += " near " + yoi::wstring2string(__current_file_path) + ":" + std::to_string(line + 1) + ":" + std::to_string(col + 1);
            message += "\n" + yoi::wstring2string(get_line_hint_for_error(__current_file_path, line + 1, col + 1));
        } else {
            message += " near line " + std::to_string(line) + " col " + std::to_string(col);
        }

        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cerr << "[hoshi-lang warning] " << message << std::endl;
    }

    /**
     * @brief Asserts a condition that would be true and throws a runtime_error if it is false.
     * 
     * @param condition the condition to be asserted.
     * @param line the line number in source code.
     * @param col the column number in source code.
     * @param msg the error message to be displayed.
     */
    void yoi_assert(bool condition, yoi::indexT line, yoi::indexT col, const std::string &msg) {
        if (not condition) {
            // throw std::runtime_error("At line " + std::to_string(line) + " col " + std::to_string(col) + ": " + msg);
            panic(line, col, msg);
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

    yoi::wstr realpath(const std::wstring &path) {
        std::wstring result;
        std::filesystem::path p(wstring2string(path));
        std::error_code ec;
        if (auto res = std::filesystem::absolute(p, ec); ec)
            throw std::runtime_error("Unable to resolve real path: [Errno " + std::to_string(ec.value()) + "]" +
                                     ec.message());
        else
            return string2wstring(res.string());
    }
    std::wstring whereIsHoshiLang() {
        std::string path;
        int length, dirnameLength;

        if (auto e = getenv("HOSHI_HOME"); e != nullptr) {
            return string2wstring(e) + L"/bin";
        } else {
            length = wai_getExecutablePath(nullptr, 0, &dirnameLength);
            path.resize(length + 1);
            wai_getExecutablePath(path.data(), length, &dirnameLength);
            path[length] = '\0';
            return yoi::string2wstring(
                path.substr(0, path.rfind(std::filesystem::path::preferred_separator)));
        }
    }
} // namespace yoi
