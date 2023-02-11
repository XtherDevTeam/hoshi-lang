#include <string>
#include <vector>
#include <sstream>

namespace hoshi {
    using wstr = std::wstring;
    using wchar = wstr::value_type;
    template<typename t>
    using vec = std::vector<t>;
    void parseString(std::wistream &input, wstr &value) ;
}