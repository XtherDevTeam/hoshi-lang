//
// Created by XIaokang00010 on 2023/2/10.
//

#include <share/def.hpp>

namespace hoshi {
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
}