#pragma once

#include <array>
#include <concepts>
#include <fstream>
#include <string>
#include <vector>

#include "vec3.hpp"

template <typename T> struct Grid3d {
    std::vector<T> data{};
    T outside{};
    int64_t width{};
    int64_t height{};
    int64_t widtheight{};
    int64_t depth{};
    const T empty{};

    constexpr bool validPos(const Vec3<int64_t> &pos) const {
        return validPos(pos.x, pos.y, pos.z);
    }
    constexpr bool validPos(const int64_t x, const int64_t y, const int64_t z) const {
        return x >= 0 and y >= 0 and z >= 0 and x < width and y < height and z < depth;
    }
    constexpr size_t index(const Vec3<int64_t> &pos) const { return index(pos.x, pos.y, pos.z); }
    constexpr size_t index(const int64_t x, const int64_t y, const int64_t z) const {
        return static_cast<size_t>(x + y * width + z * widtheight);
    }

    constexpr Grid3d() = default;
    constexpr Grid3d(const char *filename, const int64_t depth = 1, T empty = T{})
        : depth(depth), empty(empty) {
        std::ifstream infile{filename};
        std::string line;
        while (std::getline(infile, line)) {
            width = static_cast<int64_t>(line.size());
            ++height;
            for (const auto c : line) {
                data.push_back(c - '0');
            }
        }
        widtheight = width * height;
        data.reserve(data.size() * depth);
        for ([[maybe_unused]] const auto d : std::views::iota(1, depth)) {
            data.insert(data.end(), data.begin(), data.begin() + widtheight);
        }
    }
    constexpr Grid3d(int64_t width, int64_t height, int64_t depth, T value = T{}, T empty = T{})
        : width(width), height(height), widtheight(width * height), depth(depth), empty(empty) {
        data.resize(widtheight * depth, value);
    }

    // access by [x, y, z]
    constexpr const T &operator[](const int64_t x, const int64_t y, const int64_t z) const {
        if (!validPos(x, y, z)) {
            return empty;
        }
        return data[index(x, y, z)];
    }
    constexpr T &operator[](const int64_t x, const int64_t y, const int64_t z) {
        if (!validPos(x, y, z)) {
            return outside = empty;
        }
        return data[index(x, y, z)];
    }
    // access by [Vec3]
    constexpr const T &operator[](const Vec3<int64_t> p) const {
        if (!validPos(p)) {
            return empty;
        }
        return data[index(p)];
    }
    constexpr T &operator[](const Vec3<int64_t> p) {
        if (!validPos(p)) {
            return outside = empty;
        }
        return data[index(p)];
    }

    constexpr bool operator==(const Grid3d &other) const {
        if (width != other.width or height != other.height or depth != other.depth or
            empty != other.empty) {
            return false;
        }
        return data == other.data;
    }
};

// https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector/72073933#72073933
template <typename T>
concept Grid3dDataCastable32 = requires(T x) { static_cast<uint32_t>(x); };

template <Grid3dDataCastable32 T> struct std::hash<Grid3d<T>> {
    constexpr std::size_t operator()(const Grid3d<T> &g) const noexcept {
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
template <typename T> struct fmt::formatter<Grid3d<T>> {
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
    constexpr auto format(const Grid3d<T> &grid, FormatContext &ctx) const -> decltype(ctx.out()) {
        for (const auto y : std::ranges::views::iota(0, grid.height)) {
            for (const auto x : std::ranges::views::iota(0, grid.width)) {
                fmt::format_to(ctx.out(), "{}", grid[x, y]);
            }
            fmt::format_to(ctx.out(), "\n");
        }
        return fmt::format_to(ctx.out(), "");
    }
};

constexpr std::array<Vec3<int64_t>, 6> neighbours6{
    {{1, 0, 0}, {0, -1, 0}, {-1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0, 0, -1}}};

// constexpr std::array<Vec3<int64_t>, 27> neighbours27 =
//     {{1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}}};
