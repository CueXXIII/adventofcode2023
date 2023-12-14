#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

#include "grid.hpp"
#include "vec2.hpp"

using std::views::iota;

enum cardinal { east, north, west, south };

// transforms north moves to how moves
// may mirror the coordinates, which does not matter today
constexpr Vec2l transform(const enum cardinal how, const Vec2l &pos, const Vec2l &size) {
    switch (how) {
    case north:
        return pos;
    case south:
        return {pos.x, size.y - 1 - pos.y};
    case west:
        return {pos.y, pos.x};
    case east:
        return {size.y - 1 - pos.y, pos.x};
    }
    fmt::print("Unreachable enum value {}\n", how);
    return {-1, -1};
}

constexpr Vec2l transformSize(const enum cardinal how, const Vec2l &size) {
    switch (how) {
    case north:
    case south:
        return size;
    case west:
    case east:
        return {size.y, size.x};
    }
    fmt::print("Unreachable enum value {}\n", how);
    return {-1, -1};
}

template <enum cardinal how> constexpr void tilt(auto &platform) {
    const auto limit = Vec2l{platform.width, platform.height};
    const auto size = transformSize(how, limit);
    for (const auto x : iota(0, size.x)) {
        for (const auto y : iota(1, size.y)) {
            for (const auto o : iota(0, size.y)) {
                const auto from = transform(how, {x, y - o}, limit);
                const auto to = transform(how, {x, y - o - 1}, limit);
                if (platform[from] == 'O' and platform[to] == '.') {
                    platform[from] = '.';
                    platform[to] = 'O';
                } else {
                    break;
                }
            }
        }
    }
}

constexpr int64_t getWeight(const auto &platform) {
    int64_t sum = 0;
    for (const auto x : iota(0, platform.width)) {
        for (const auto y : iota(0, platform.height)) {
            if (platform[x, y] == 'O') {
                sum += platform.height - y;
            }
        }
    }
    return sum;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    Grid<char> platform{argv[1]};

    std::unordered_map<Grid<char>, int64_t> cycle{};
    std::unordered_map<int64_t, int64_t> weights{};

    tilt<north>(platform);
    fmt::print("{}", platform);
    fmt::print("The platform needs to support {} metric O\n", getWeight(platform));

    // complete cycle 1
    tilt<west>(platform);
    tilt<south>(platform);
    tilt<east>(platform);
    cycle[platform] = 1;
    weights[1] = getWeight(platform);

    const int64_t cyclesTotal = 1000000000;
    for ([[maybe_unused]] const auto n : iota(2, cyclesTotal + 1)) {
        tilt<north>(platform);
        tilt<west>(platform);
        tilt<south>(platform);
        tilt<east>(platform);
        if (cycle.contains(platform)) {
            const auto cStart = cycle[platform];
            const auto cLen = n - cStart;
            const auto cLeft = (cyclesTotal - cStart) % cLen;
            fmt::print("Detected cycle from {} to {}\n", cStart, n);
            // fmt::print("{}", platform);
            fmt::print("After {} cycles there are {} metric O\n", cyclesTotal,
                       weights[cStart + cLeft]);
            break;
        } else {
            cycle[platform] = n;
            weights[n] = getWeight(platform);
        }
    }
}
