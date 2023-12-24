#pragma once

#include <array>
#include <concepts>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "vec2.hpp"

template <typename T> struct Grid {
    std::vector<T> data{};
    T outside{};
    int64_t width{};
    int64_t height{};
    T const empty{};

    constexpr bool validPos(Vec2<int64_t> const &pos) const { return validPos(pos.x, pos.y); }
    constexpr bool validPos(int64_t const x, int64_t const y) const {
        return x >= 0 and y >= 0 and x < width and y < height;
    }
    constexpr size_t index(Vec2<int64_t> const &pos) const { return index(pos.x, pos.y); }
    constexpr size_t index(int64_t const x, int64_t const y) const {
        return static_cast<size_t>(x + y * width);
    }

    constexpr Grid() = default;
    constexpr Grid(char const *filename, T const empty = T{}) : empty(empty) {
        std::ifstream infile{filename};
        std::string line;
        while (std::getline(infile, line)) {
            width = static_cast<int64_t>(line.size());
            height++;
            // data.append_range(line);
            data.insert(data.end(), line.begin(), line.end());
        }
    }
    constexpr Grid(int64_t const width, int64_t const height, T const value = T{},
                   T const empty = T{})
        : width(width), height(height), empty(empty) {
        data.resize(width * height, value);
    }

    // access by [x, y]
    constexpr T const &operator[](int64_t const x, int64_t const y) const {
        if (!validPos(x, y)) {
            return empty;
        }
        return data[index(x, y)];
    }
    constexpr T &operator[](int64_t const x, int64_t const y) {
        if (!validPos(x, y)) {
            return outside = empty;
        }
        return data[index(x, y)];
    }
    // access by [Vec2]
    constexpr T const &operator[](Vec2<int64_t> const p) const {
        if (!validPos(p)) {
            return empty;
        }
        return data[index(p)];
    }
    constexpr T &operator[](Vec2<int64_t> const p) {
        if (!validPos(p)) {
            return outside = empty;
        }
        return data[index(p)];
    }

    constexpr bool operator==(Grid const &other) const {
        if (width != other.width or height != other.height or empty != other.empty) {
            return false;
        }
        return data == other.data;
    }

    void print() const {
        for (auto const y : std::views::iota(0, height)) {
            for (auto const x : std::views::iota(0, width)) {
                std::cout << (*this)[x, y];
            }
            std::cout << '\n';
        }
    }
};

// https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector/72073933#72073933
template <typename T>
concept GridDataCastable32 = requires(T x) { static_cast<uint32_t>(x); };

template <GridDataCastable32 T> struct std::hash<Grid<T>> {
    constexpr std::size_t operator()(Grid<T> const &g) const noexcept {
        std::size_t seed = g.data.size();
        for (auto const xval : g.data) {
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
    constexpr auto format(Grid<T> const &grid, FormatContext &ctx) const -> decltype(ctx.out()) {
        for (auto const y : std::ranges::views::iota(0, grid.height)) {
            for (auto const x : std::ranges::views::iota(0, grid.width)) {
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
