#pragma once

#include <iterator>

#include "vec2.hpp"
#include "vec3.hpp"

template <typename VEC> class ScanLocations {
    const VEC position{};
    const VEC direction{};
    const VEC minCorner{};
    const VEC maxCorner{};

  public:
    ScanLocations(const VEC &position, const VEC &direction, const VEC &minCorner,
                  const VEC &maxCorner)
        : position(position), direction(direction), minCorner(minCorner), maxCorner(maxCorner) {}

    class sentinel {};
    class iterator {
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using pointer = VEC *;
        using reference = VEC &;
        using value_type = VEC;

        VEC current;
        const ScanLocations &limits;

        template <typename T> constexpr bool isBelow(const Vec2<T> &a, const Vec2<T> &b) const {
            return a.x < b.x or a.y < b.y;
        }

        template <typename T> constexpr bool isBelow(const Vec3<T> &a, const Vec3<T> &b) const {
            return a.x < b.x or a.y < b.y or a.z < b.z;
        }

      public:
        iterator(const ScanLocations &scanner) : current(scanner.position), limits(scanner) {}
        bool operator==(sentinel) const {
            return isBelow(current, limits.minCorner) or isBelow(limits.maxCorner, current);
        }
        iterator &operator++() {
            current += limits.direction;
            return *this;
        }
        VEC &operator*() { return current; }
    };

    iterator begin() const { return {*this}; }
    sentinel end() const { return {}; }
};
