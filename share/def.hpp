#ifndef HOSHI_DEF_HPP
#define HOSHI_DEF_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <stack>
#include <memory>
#include <share/utfutils.hpp>

namespace yoi {
    using wstr = std::wstring;
    using wchar = wstr::value_type;
    using vdeci = double;
    using indexT = uint64_t;
    template<typename t>
    using vec = std::vector<t>;

    void parseString(std::wistream &input, wstr &value);

    void panic(yoi::indexT line, yoi::indexT col, const std::string &msg);

    void assert(bool cond, yoi::indexT line, yoi::indexT col, const std::string &msg);

    std::wstring string2wstring(const std::string &v);

    std::string wstring2string(const std::wstring &v);

    template<typename A, typename B>
    class indexTable {
        vec<std::pair<A, B>> indexes;
    public:

        yoi::indexT put(const A &a, const B &b) {
            for (yoi::indexT i = 0;i < indexes.size();i++)
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

        B &operator[](yoi::indexT k) {
            if (k < indexes.size())
                return indexes[k].second;
            else
                throw std::runtime_error("indexTable: invalid index");
        }

        yoi::indexT getIndex(const A& k) {
            for (yoi::indexT i = 0;i < indexes.size();i++)
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
        yoi::indexT put(const T &t) {
            yoi::indexT i = 0;
            for (; i < pool.size(); i++)
                if (pool[i] == t)
                    return i;
            pool.push_back(t);
            return i;
        }

        T &operator[](yoi::indexT i) {
            if (i < pool.size())
                return pool[i];
            else
                throw std::runtime_error("indexPool: invalid index");
        }
    };

    template<typename T>
    std::shared_ptr<T> managedPtr(const T &v) {
        return std::make_shared<T>(v);
    }

    yoi::wstr realpath(const std::wstring &path);
}
#endif