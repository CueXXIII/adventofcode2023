#pragma once

#include "vec2.hpp"
#include "vec3.hpp"

// Manhattan distance for vectors
template <typename T> constexpr T manhattan(Vec2<T> const &from, Vec2<T> const &to) {
    return std::abs(from.x - to.x) + std::abs(from.y - to.y);
}

template <typename T> constexpr T manhattan(Vec3<T> const &from, Vec3<T> const &to) {
    return std::abs(from.x - to.x) + std::abs(from.y - to.y) + std::abs(from.z - to.z);
}

// signum for numbers
template <typename T> constexpr T signum(T const num) { return (T{0} < num) - (num < T{0}); }

// signum for vectors: creates a signum per dimension
template <typename T> constexpr Vec2<T> signum(Vec2<T> const &vec) {
    return {signum(vec.x), signum(vec.y)};
}

template <typename T> constexpr Vec3<T> signum(Vec3<T> const &vec) {
    return {signum(vec.x), signum(vec.y), signum(vec.z)};
}
