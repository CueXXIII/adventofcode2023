#pragma once

#include <array>
#include <fstream>
#include <string>
#include <vector>

#include "vec2.hpp"

template <typename T> struct Grid {
    std::vector<T> data{};
    T outside{};
    int64_t width{};
    int64_t height{};
    const T empty{};

    inline bool validPos(const Vec2<int64_t> &pos) const { return validPos(pos.x, pos.y); }
    inline bool validPos(const int64_t x, const int64_t y) const {
        return x >= 0 and y >= 0 and x < width and y < height;
    }
    inline size_t index(const Vec2<int64_t> &pos) const { return index(pos.x, pos.y); }
    inline size_t index(const int64_t x, const int64_t y) const {
        return static_cast<size_t>(x + y * width);
    }

    Grid() = default;
    Grid(const char *filename, T empty = T{}) : empty(empty) {
        std::ifstream infile{filename};
        std::string line;
        while (std::getline(infile, line)) {
            width = line.size();
            height++;
            // data.append_range(line);
            data.insert(data.end(), line.begin(), line.end());
        }
    }
    Grid(int64_t width, int64_t height, T value = T{}, T empty = T{})
        : width(width), height(height), empty(empty) {
        data.resize(width * height, value);
    }

    // access by [x, y]
    const T &operator[](const int64_t x, const int64_t y) const {
        if (!validPos(x, y)) {
            return empty;
        }
        return data[index(x, y)];
    }
    T &operator[](const int64_t x, const int64_t y) {
        if (!validPos(x, y)) {
            return outside = empty;
        }
        return data[index(x, y)];
    }
    // access by [Vec2]
    const T &operator[](const Vec2<int64_t> p) const {
        if (!validPos(p)) {
            return empty;
        }
        return data[index(p)];
    }
    T &operator[](const Vec2<int64_t> p) {
        if (!validPos(p)) {
            return outside = empty;
        }
        return data[index(p)];
    }
};

constexpr std::array<Vec2<int64_t>, 4> neighbours4{{{1, 0}, {0, -1}, {-1, 0}, {0, 1}}};

constexpr std::array<Vec2<int64_t>, 8> neighbours8{
    {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}}};
