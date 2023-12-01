#pragma once

#include <algorithm>
#include <concepts>
#include <fstream>

#include "vec2.hpp"

template <typename num> struct Vec3 {
    num x{};
    num y{};
    num z{};

    constexpr Vec3() noexcept = default;
    constexpr Vec3(const num &x, const num &y, const num &z) noexcept : x(x), y(y), z(z) {}
    constexpr Vec3(const Vec2<num> &v, num z) noexcept : x(v.x), y(v.y), z(z) {}

    constexpr bool operator==(const Vec3 &other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    constexpr Vec3 &operator+=(const Vec3 &other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    constexpr Vec3 &operator-=(const Vec3 &other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
    constexpr Vec3 &operator*=(const num factor) {
        x *= factor;
        y *= factor;
        z *= factor;
        return *this;
    }
    constexpr Vec3 &operator/=(const num divisor) {
        x /= divisor;
        y /= divisor;
        z /= divisor;
        return *this;
    }

    constexpr Vec3 operator+(const Vec3 &other) const { return Vec3{*this} += other; }
    constexpr Vec3 operator-(const Vec3 &other) const { return Vec3{*this} -= other; }
    constexpr Vec3 operator*(const num factor) const { return Vec3{*this} *= factor; }
    constexpr Vec3 operator/(const num divisor) const { return Vec3{*this} /= divisor; }

    friend constexpr std::ostream &operator<<(std::ostream &out, const Vec3 &vec) {
        return out << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    }
};

template <typename num> struct std::hash<Vec3<num>> {
    constexpr std::size_t operator()(const Vec3<num> &v) const noexcept {
        return std::hash<num>{}(v.x) * 2 + std::hash<num>{}(v.y) * 1627 +
               std::hash<num>{}(v.z) * 2642257;
    }
};

// from https://fmt.dev/latest/api.html#format-api
template <typename num> struct fmt::formatter<Vec3<num>> {
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
    constexpr auto format(const Vec3<num> &vec, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "({}, {}, {})", vec.x, vec.y, vec.z);
    }
};

// is_instantiation_of isdefined and included from vec2.hpp

// Return bounding box as std::pair<min, max> over an iterable container.
// The container must not be empty.
template <typename iter>
concept isVec3Iterable = is_instantiation_of<Vec3, typename iter::value_type>::value;

template <typename iterable>
constexpr auto boundingBox(iterable &container) requires isVec3Iterable<iterable> {
    auto it = container.begin();
    auto min{*it};
    auto max{*it};
    while (++it != container.end()) {
        min = {std::min(min.x, it->x), std::min(min.y, it->y), std::min(min.z, it->z)};
        max = {std::max(max.x, it->x), std::max(max.y, it->y), std::max(max.z, it->z)};
    }
    return std::pair{min, max};
}

using Vec3i = Vec3<int32_t>;
using Vec3l = Vec3<int64_t>;
using Vec3z = Vec3<size_t>;
using Vec3f = Vec3<double>;
