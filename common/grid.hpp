#pragma once

#include <array>
#include <concepts>
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

    bool operator==(const Grid &other) const {
        if (width != other.width or height != other.height or empty != other.empty) {
            return false;
        }
        return data == other.data;
    }
};

// https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector/72073933#72073933
template <typename T>
concept GridDataCastable32 = requires(T x) { static_cast<uint32_t>(x); };

template <GridDataCastable32 T> struct std::hash<Grid<T>> {
    constexpr std::size_t operator()(const Grid<T> &g) const noexcept {
        std::size_t seed = g.data.size();
        for (const auto xval : g.data) {
            auto x = static_cast<uint32_t>(xval);
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = (x >> 16) ^ x;
            seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

// from https://fmt.dev/latest/api.html#format-api
template <typename T> struct fmt::formatter<Grid<T>> {
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
    constexpr auto format(const Grid<T> &grid, FormatContext &ctx) const -> decltype(ctx.out()) {
        for (const auto y : std::ranges::views::iota(0, grid.height)) {
            for (const auto x : std::ranges::views::iota(0, grid.width)) {
                fmt::format_to(ctx.out(), "{}", grid[x, y]);
            }
            fmt::format_to(ctx.out(), "\n");
        }
        return fmt::format_to(ctx.out(), "");
    }
};

constexpr std::array<Vec2<int64_t>, 4> neighbours4{{{1, 0}, {0, -1}, {-1, 0}, {0, 1}}};

constexpr std::array<Vec2<int64_t>, 8> neighbours8{
    {{1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}}};
