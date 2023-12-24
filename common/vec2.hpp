#pragma once

#include <algorithm>
#include <concepts>
#include <fmt/format.h>
#include <fstream>

template <typename num> struct Vec2 {
    num x{};
    num y{};

    constexpr Vec2() noexcept = default;
    constexpr Vec2(num const x, num const y) noexcept : x(x), y(y) {}

    constexpr bool operator==(Vec2 const &other) const { return x == other.x && y == other.y; }
    constexpr Vec2 &operator+=(Vec2 const &other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    constexpr Vec2 &operator-=(Vec2 const &other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    constexpr Vec2 &operator*=(num const factor) {
        x *= factor;
        y *= factor;
        return *this;
    }

    constexpr Vec2 operator+(Vec2 const &other) const { return Vec2{*this} += other; }
    constexpr Vec2 operator-(Vec2 const &other) const { return Vec2{*this} -= other; }
    constexpr Vec2 operator*(num const factor) const { return Vec2{*this} *= factor; }

    constexpr auto operator<=>(Vec2 const &other) const {
        return std::pair(x, y) <=> std::pair(other.x, other.y);
    }

    friend constexpr std::ostream &operator<<(std::ostream &out, Vec2 const &vec) {
        return out << "(" << vec.x << ", " << vec.y << ")";
    }
};

template <typename num> struct std::hash<Vec2<num>> {
    constexpr std::size_t operator()(Vec2<num> const &v) const noexcept {
        return std::hash<int64_t>{}(
            (static_cast<int64_t>(v.x) << 16 ^ static_cast<int64_t>(v.x) >> 48) ^
            static_cast<int64_t>(v.y));
    }
};

// from https://fmt.dev/latest/api.html#format-api
template <typename num> struct fmt::formatter<Vec2<num>> {
    // parse format specifier "...{:3.14f}..." is passed as "3.14f}..."
    // and it must advance to the }
    // TODO: save custom format and use them for all values
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}')
            throw format_error("invalid format");
        return it;
    }
    template <typename FormatContext>
    constexpr auto format(Vec2<num> const &vec, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "({}, {})", vec.x, vec.y);
    }
};

// from
// https://stackoverflow.com/questions/17390605/doing-a-static-assert-that-a-template-type-is-another-template
template <template <typename...> class T, typename... Ts>
struct is_instantiation_of : std::false_type {};
template <template <typename...> class T, typename... Ts>
struct is_instantiation_of<T, T<Ts...>> : std::true_type {};

// Return bounding box as std::pair<min, max> over an iterable container.
// The container must not be empty.
template <typename iter>
concept isVec2Iterable = is_instantiation_of<Vec2, typename iter::value_type>::value;

template <typename iterable>
constexpr auto boundingBox(iterable const &container)
    requires isVec2Iterable<iterable>
{
    auto it = container.begin();
    auto min{*it};
    auto max{*it};
    while (++it != container.end()) {
        min = {std::min(min.x, it->x), std::min(min.y, it->y)};
        max = {std::max(max.x, it->x), std::max(max.y, it->y)};
    }
    return std::pair{min, max};
}

using Vec2i = Vec2<int32_t>;
using Vec2l = Vec2<int64_t>;
using Vec2z = Vec2<size_t>;
using Vec2f = Vec2<double>;
