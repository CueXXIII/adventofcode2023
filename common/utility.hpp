#pragma once

#include "vec2.hpp"
#include "vec3.hpp"

// Manhattan distance for vectors
template <typename T> constexpr T manhattan(const Vec2<T> &from, const Vec2<T> &to) {
    return std::abs(from.x - to.x) + std::abs(from.y - to.y);
}

template <typename T> constexpr T manhattan(const Vec3<T> &from, const Vec3<T> &to) {
    return std::abs(from.x - to.x) + std::abs(from.y - to.y) + std::abs(from.z - to.z);
}

// signum for numbers
template <typename T> constexpr T signum(const T num) {
    return (T{0} < num) - (num < T{0});
}

// signum for vectors: creates a signum per dimension
template <typename T> constexpr Vec2<T> signum(const Vec2<T> &vec) {
    return {signum(vec.x), signum(vec.y)};
}

template <typename T> constexpr Vec3<T> signum(const Vec3<T> &vec) {
    return {signum(vec.x), signum(vec.y), signum(vec.z)};
}
