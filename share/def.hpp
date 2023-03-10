#include <string>
#include <vector>
#include <sstream>
#include <share/utfutils.hpp>

namespace hoshi {
    using wstr = std::wstring;
    using wchar = wstr::value_type;
    using vdeci = double;
    template<typename t>
    using vec = std::vector<t>;

    void parseString(std::wistream &input, wstr &value);

    void panic(int64_t line, int64_t col, const std::string &msg);

    std::wstring string2wstring(const std::string &v);

    std::string wstring2string(const std::wstring &v);

    template<typename A, typename B>
    class indexTable {
        vec<std::pair<A, B>> indexes;
    public:

        int64_t put(const A &a, const B &b) {
            for (int64_t i = 0;i < indexes.size();i++)
                if (indexes[i].first == a) {
                    indexes[i].second = b;
                    return i;
                }
            indexes.push_back({a, b});
            return indexes.size();
        }

        B &operator[](const A &k) {
            for (auto &i: indexes) {
                if (i.first == k)
                    return i.second;
            }
            throw std::runtime_error("indexTable: invalid index");
        }

        B &operator[](int64_t k) {
            if (k < indexes.size())
                return indexes[k].second;
            else
                throw std::runtime_error("indexTable: invalid index");
        }

        int64_t getIndex(const A& k) {
            for (int64_t i = 0;i < indexes.size();i++)
                if (indexes[i].first == k) {
                    return i;
                }
            throw std::runtime_error("indexTable: invalid key");
        }
    };

    template<typename T>
    class indexPool {
        vec<T> pool;
    public:
        int64_t put(const T &t) {
            int64_t i = 0;
            for (; i < pool.size(); i++)
                if (pool[i] == t)
                    return i;
            pool.push_back(t);
            return i;
        }

        T &operator[](int64_t i) {
            if (i < pool.size())
                return pool[i];
            else
                throw std::runtime_error("indexPool: invalid index");
        }

    };
}