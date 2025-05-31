/*
MIT License

Copyright(c) 2021 Futurewei Cloud

    Permission is hereby granted,
    free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in all copies
    or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS",
    WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER
    LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#pragma once
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <fmt/ranges.h>
#include <k2/logging/AutoGenFormattingUtils.h>
#include <k2/logging/Decimal.h>

#include <iostream>
#include <optional>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/multiprecision/cpp_dec_float.hpp>

namespace k2 {
// helper function for converting enum class into an integral type
// e.g. usage: auto integralColor = to_integral(MyEnum::Red);
// or  std::array<MyEnum, to_integral(MyEnum::Red)>;
template <typename T>
inline auto to_integral(T e) { return static_cast<std::underlying_type_t<T>>(e); }

// base case recursion call
inline void hash_combine_seed(size_t&) {
}

// hash-combine hashes for multiple objects over a seed
// this is using boost-like hash combination
// https://stackoverflow.com/questions/35985960/c-why-is-boosthash-combine-the-best-way-to-combine-hash-values
template <typename T, typename... Rest>
inline void hash_combine_seed(size_t& seed, const T& v, Rest&&... rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine_seed(seed, std::forward<Rest>(rest)...);
}

// hash-combine multiple objects
template <typename T, typename... Rest>
inline size_t hash_combine(const T& v, Rest&&... rest) {
    size_t seed = 0;
    hash_combine_seed(seed, v, std::forward<Rest>(rest)...);
    return seed;
}

// return the string version of a type
template <typename T>
constexpr auto type_name() {
    std::string_view name, prefix, suffix;
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto type_name() [with T = ";
    suffix = "]";
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

} // ns k2

// Intrusive formatting for classes
// e.g. usage:
// namespace k2::dto {
// struct GetSchemaRequest() {
//    String schemaName;
//    K2_DEF_FMT(schemaName);
// };
// }


// --- Trait to check for iterability (excluding strings) ---
template <typename T, typename = void>
struct is_iterable : std::false_type {};

template <typename T>
struct is_iterable<
    T,
    std::void_t<decltype(std::begin(std::declval<T>())),
                decltype(std::end(std::declval<T>()))>
> : std::true_type {};

template <typename T>
constexpr bool is_iterable_v = is_iterable<T>::value &&
    !std::is_convertible_v<T, std::string>; // exclude std::string since we don't want individual characters

// --- joinable_range wrapper for iterable types ---
template <typename T>
struct joinable_range {
    const T& ref;
};

template <typename T>
struct non_joinable_range {
    const T& ref;
};

template <typename T>
struct fmt::formatter<joinable_range<T>> {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const joinable_range<T>& jr, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), fmt::runtime("{}"), fmt::join(jr.ref, ", "));
    }
};


template <typename T>
struct fmt::formatter<non_joinable_range<T>> {
    constexpr auto parse(fmt::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const non_joinable_range<T>& nr, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), fmt::runtime("{}"), nr.ref);
    }
};

// --- Transparent joinable() overloads ---
template <typename T>
constexpr auto joinable(const T& t) {
    if constexpr (is_iterable_v<T>) {
        return joinable_range<T>{t}; // iterable: wrap
    } else {
        return non_joinable_range<T>{t}; // non-iterable: passthrough
    }
}

#define K2_DEF_FMT(_K2_CTYPE_ARG, ...)                                                        \
    template <typename FormatContext>                                                         \
    static auto ___K2___INTERNAL_fmt_helper(const _K2_CTYPE_ARG &o, FormatContext &ctx)       \
    {                                                                                         \
        return fmt::format_to(ctx.out(),                                                      \
                              FMT_STRING("{{" _K2_MKLIST(__VA_ARGS__) "}}") _K2_MKVARS(__VA_ARGS__)); \
    }

template <typename T, typename FormatContext, typename = void>
struct ___K2___INTERNAL_IsK2Formattable : std::false_type {};

template <typename T, typename FormatContext>
struct ___K2___INTERNAL_IsK2Formattable<T, FormatContext,
    std::void_t<decltype(T::___K2___INTERNAL_fmt_helper(std::declval<const T&>(), std::declval<FormatContext&>()))>
> : std::true_type {};

namespace fmt {
    template <typename T, typename Char>
    struct formatter<T, Char, std::enable_if_t<
        ___K2___INTERNAL_IsK2Formattable<T, fmt::format_context>::value
    >> {
        constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const T& obj, FormatContext& ctx) const {
            return T::___K2___INTERNAL_fmt_helper(obj, ctx);
        }
    };
}


// Generating enums with formatting. the _IC version is to be used when inside a class.
// e.g.
// namespace k2::dto {
// K2_DEF_ENUM(TxnStatus, Created, Aborted, Committed) )
// }
#define K2_DEF_ENUM_IC(_K2_ENUM_TYPE_NAME, ...)                                                   \
    enum class _K2_ENUM_TYPE_NAME                                                                 \
    {                                                                                             \
        __VA_ARGS__                                                                               \
    };                                                                                            \
    inline static const char *const _K2_ENUM_TYPE_NAME##Names[] = {                               \
        _K2_TO_STRING_LIST(__VA_ARGS__)};                                                         \
    inline static _K2_ENUM_TYPE_NAME _K2_ENUM_TYPE_NAME##FromStr(const std::string &str)          \
    {                                                                                             \
        _K2_ENUM_IF_STMT(_K2_ENUM_TYPE_NAME, ##__VA_ARGS__);                                      \
        std::string s = fmt::format("unsupported value:{} in enum {}", str, #_K2_ENUM_TYPE_NAME); \
        throw std::runtime_error(s.c_str());                                                      \
    }                                                                                             \
    template <typename OStream_T>                                                                 \
    friend inline OStream_T &operator<<(OStream_T &os, const _K2_ENUM_TYPE_NAME &o)               \
    {                                                                                             \
        return os << "{" << _K2_ENUM_TYPE_NAME##Names[k2::to_integral(o)] << "}";                 \
    }

#define K2_DEF_ENUM(_K2_ENUM_TYPE_NAME, ...)                                                      \
    enum class _K2_ENUM_TYPE_NAME                                                                 \
    {                                                                                             \
        __VA_ARGS__                                                                               \
    };                                                                                            \
    inline static const char *const _K2_ENUM_TYPE_NAME##Names[] = {                               \
        _K2_TO_STRING_LIST(__VA_ARGS__)};                                                         \
    inline static _K2_ENUM_TYPE_NAME _K2_ENUM_TYPE_NAME##FromStr(const std::string &str)          \
    {                                                                                             \
        _K2_ENUM_IF_STMT(_K2_ENUM_TYPE_NAME, ##__VA_ARGS__);                                      \
        std::string s = fmt::format("unsupported value:{} in enum {}", str, #_K2_ENUM_TYPE_NAME); \
        throw std::runtime_error(s.c_str());                                                      \
    }                                                                                             \
    template <typename OStream_T>                                                                 \
    inline OStream_T &operator<<(OStream_T &os, const _K2_ENUM_TYPE_NAME &o)                      \
    {                                                                                             \
        return os << "{" << _K2_ENUM_TYPE_NAME##Names[k2::to_integral(o)] << "}";                 \
    }

// enable ostream formatter for enum types
template <typename T, typename Char>
struct fmt::formatter<T, Char, std::enable_if_t<std::is_enum_v<T>>> : fmt::ostream_formatter {};

template <>
struct fmt::formatter<k2::DecimalD25> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin();}

    template <typename FormatContext>
    auto format(k2::DecimalD25 const& d, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", d.str());
    }
};

template <>
struct fmt::formatter<k2::DecimalD50> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin();}

    template <typename FormatContext>
    auto format(k2::DecimalD50 const& d, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", d.str());
    }
};

template <>
struct fmt::formatter<k2::DecimalD100> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin();}

    template <typename FormatContext>
    auto format(k2::DecimalD100 const& d, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", d.str());
    }
};

#include <memory>

// Generic formatter for std::shared_ptr<T>
template <typename T, typename Char>
struct fmt::formatter<std::shared_ptr<T>, Char> : fmt::formatter<T, Char> {
    template <typename FormatContext>
    auto format(const std::shared_ptr<T>& sptr, FormatContext& ctx) {
        if (sptr) {
            // format the pointed-to object
            return fmt::formatter<T, Char>::format(*sptr, ctx);
        } else {
            // handle null pointer case
            return fmt::format_to(ctx.out(), "{{null}}");
        }
    }
};

// provide support for formatting of stl containers of bool type
template <>  // fmt support
struct fmt::formatter<std::set<bool>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(std::set<bool> const& strset, FormatContext& ctx) {
        fmt::format_to(ctx.out(), "{{");
        size_t processed = 0;
        for (auto it = strset.cbegin(); it != strset.cend(); ++it) {
            if (processed == strset.size() - 1) {
                fmt::format_to(ctx.out(), "{}", *it);
            } else {
                fmt::format_to(ctx.out(), "{}, ", *it);
            }
        }
        return fmt::format_to(ctx.out(), "}}");
    }
};


template <>  // fmt support
struct fmt::formatter<std::unordered_set<bool>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(std::unordered_set<bool> const& strset, FormatContext& ctx) {
        fmt::format_to(ctx.out(), "{{");
        size_t processed = 0;
        for (auto it = strset.cbegin(); it != strset.cend(); ++it) {
            if (processed == strset.size() - 1) {
                fmt::format_to(ctx.out(), "{}", *it);
            } else {
                fmt::format_to(ctx.out(), "{}, ", *it);
            }
        }
        return fmt::format_to(ctx.out(), "}}");
    }
};

template <typename T>  // fmt support for optionals
struct fmt::formatter<std::optional<T>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(std::optional<T> const& opt, FormatContext& ctx) {
        fmt::format_to(ctx.out(), "{{");
        if (opt.has_value()) {
            fmt::format_to(ctx.out(), "{}", *opt);
        }
        return fmt::format_to(ctx.out(), "}}");
    }
};
