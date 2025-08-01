#ifndef HOSHI_DEF_HPP
#define HOSHI_DEF_HPP

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>
#include <stack>
#include <memory>
#include "magic_enum.h"
#include <share/utfutils.hpp>
#include <share/whereami.h>
#include <filesystem>

#if defined(__linux__)
#define YOI_PLATFORM "linux"
#define YOI_DYLIB_SUFFIX "so"
#elif defined(__APPLE__)
#define YOI_PLATFORM "darwin"
#define YOI_DYLIB_SUFFIX L"dylib"
#elif defined(_WIN32)
#pragma comment(lib, "ws2_32.lib")
#define YOI_PLATFORM "win32"
#define YOI_DYLIB_SUFFIX "dll"
#else
#define YOI_PLATFORM "unknown"
#define YOI_DYLIB_SUFFIX "so"
#endif

#if defined(__aarch64__)
#define YOI_ARCH "arm64"
#elif defined(__x86_64__)
#define YOI_ARCH "amd64"
#elif defined(__i386__)
#define YOI_ARCH "i386"
#elif defined(__arm__)
#define YOI_ARCH "arm"
#else
#define YOI_ARCH "unknown"
#endif

namespace yoi {
    using wstr = std::wstring;
    using wchar = wstr::value_type;
    using vdeci = double;
    using indexT = uint64_t;
    template<typename t>
    using vec = std::vector<t>;

    void parseString(std::wistream &input, wstr &value);

    void panic(yoi::indexT line, yoi::indexT col, const std::string &msg);

    void warning(yoi::indexT line, yoi::indexT col, const std::string &msg);

    void yoi_assert(bool cond, yoi::indexT line, yoi::indexT col, const std::string &msg);

    std::wstring string2wstring(const std::string &v);

    std::string wstring2string(const std::wstring &v);

    std::wstring whereIsHoshiLang();

    template<typename A, typename B>
    class indexTableDeprecated {
        vec<std::pair<A, B>> indexes;
    public:

        yoi::indexT put(const A &a, const B &b) {
            for (yoi::indexT i = 0;i < indexes.size();i++)
                if (indexes[i].first == a) {
                    indexes[i].second = b;
                    return i;
                }
            indexes.push_back({a, b});
            return indexes.size() - 1;
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

        class iterator {
            typename vec<std::pair<A, B>>::iterator it;
        public:
            iterator(typename vec<std::pair<A, B>>::iterator it) : it(it) {}

            bool operator!=(const iterator &other) const { return it!= other.it; }

            iterator &operator++() { ++it; return *this; }

            std::pair<A, B> &operator*() { return *it; }
        };

        iterator begin() { return iterator(indexes.begin()); }

        iterator end() { return iterator(indexes.end()); }

        yoi::indexT size() const { return indexes.size(); }
    };

    template<typename A, typename B>
    class indexTable {
        std::map<A, yoi::indexT> indexes;
        std::vector<std::pair<A, B>> values;
    public:
        yoi::indexT put(const A &a, const B &b) {
            if (auto it = indexes.find(a); it == indexes.end()) {
                indexes[a] = values.size();
                values.push_back({a, b});
                return values.size() - 1;
            } else {
                values[it->second].second = b;
                return it->second;
            }
        }
        yoi::indexT put_create(const A &a, const B &b) {
            if (auto it = indexes.find(a); it == indexes.end()) {
                indexes[a] = values.size();
                values.push_back({a, b});
                return values.size() - 1;
            } else {
                throw std::out_of_range("indexTable: key already exists");
            }
        }
        B &operator[](const A &k) {
            if (auto it = indexes.find(k); it == indexes.end()) {
                throw std::out_of_range("indexTableRefactored: invalid key");
            } else {
                return values[it->second].second;
            }
        }

        B &operator[](yoi::indexT k) {
            if (k < indexes.size()) {
                return values[k].second;
            } else {
                throw std::out_of_range("indexTableRefactored: invalid index");
            }
        }
        yoi::indexT getIndex(const A &k) {
            if (auto it = indexes.find(k); it == indexes.end()) {
                throw std::out_of_range("indexTableRefactored: invalid key");
            } else {
                return it->second;
            }
        }
        const A& getKey(yoi::indexT i) const {
            if (i < indexes.size()) {
                return values[i].first;
            } else {
                throw std::out_of_range("indexTableRefactored: invalid index");
            }
        }
        yoi::indexT size() {
            return values.size();
        }
        // iterate over all values
        class iterator {
            typename std::vector<std::pair<A, B>>::iterator it;
        public:
            iterator(typename std::vector<std::pair<A, B>>::iterator it) : it(it) {}
            bool operator!=(const iterator &other) const { return it!= other.it; }
            iterator &operator++() { ++it; return *this; }
            std::pair<A, B> &operator*() { return *it; }
            iterator operator+(yoi::indexT i) const {
                return iterator(it + i);
            }
            std::pair<A, B> *operator->() {
                return &(*it);
            }
        };

        iterator begin() { return iterator(values.begin()); }

        iterator end() { return iterator(values.end()); }

        yoi::indexT size() const { return values.size(); }

        bool contains(const A &k) const {
            return indexes.find(k)!= indexes.end();
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
        yoi::indexT size() const { return pool.size(); }
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

    template <typename enumT, enumT TSize = enumT::FINAL>
    class [[nodiscard]] enum_range final {
        using type = std::underlying_type_t<enumT>;

        public:
        // The iterator that can be used to loop through all values
        //
        class [[nodiscard]] iterator final {
            enumT value{static_cast<enumT>(0)};

            public:
            constexpr iterator() noexcept = default;
            constexpr iterator(enumT e) noexcept : value{e} {}

            constexpr auto operator*() const noexcept -> enumT { return value; }
            constexpr auto operator-> () const & noexcept -> const enumT* {
                return &value;
            }
            constexpr auto operator++() & noexcept -> iterator {
                value = static_cast<enumT>(1 + static_cast<type>(value));
                return *this;
            }

            [[nodiscard]] constexpr auto operator==(iterator i) -> bool { return i.value == value; }
            [[nodiscard]] constexpr auto operator!=(iterator i) -> bool { return i.value != value; }
        };

        constexpr auto begin() const noexcept -> iterator { return iterator{}; }
        constexpr auto cbegin() const noexcept -> iterator { return iterator{}; }

        constexpr auto end() const noexcept -> iterator { return iterator{TSize}; }
        constexpr auto cend() const noexcept -> iterator { return iterator{TSize}; }

        [[nodiscard]] constexpr auto size() const noexcept -> type {
            return static_cast<type>(TSize);
        }
    };
}
#endif