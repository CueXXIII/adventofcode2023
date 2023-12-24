#pragma once

#include <iterator>

#include "vec2.hpp"
#include "vec3.hpp"

template <typename VEC> class ScanLocations {
    VEC const position{};
    VEC const direction{};
    VEC const minCorner{};
    VEC const maxCorner{};

  public:
    ScanLocations(VEC const &position, VEC const &direction, VEC const &minCorner,
                  VEC const &maxCorner)
        : position(position), direction(direction), minCorner(minCorner), maxCorner(maxCorner) {}

    class sentinel {};
    class iterator {
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using pointer = VEC *;
        using reference = VEC &;
        using value_type = VEC;

        VEC current;
        ScanLocations const &limits;

        template <typename T> constexpr bool isBelow(Vec2<T> const &a, Vec2<T> const &b) const {
            return a.x < b.x or a.y < b.y;
        }

        template <typename T> constexpr bool isBelow(Vec3<T> const &a, Vec3<T> const &b) const {
            return a.x < b.x or a.y < b.y or a.z < b.z;
        }

      public:
        iterator(ScanLocations const &scanner) : current(scanner.position), limits(scanner) {}
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
