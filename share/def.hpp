#include <string>
#include <vector>
#include <sstream>

namespace hoshi {
    using wstr = std::wstring;
    using wchar = wstr::value_type;
    template<typename t>
    using vec = std::vector<t>;
    void parseString(std::wistream &input, wstr &value) ;

    void panic(int64_t line, int64_t col, const std::string &msg);
}